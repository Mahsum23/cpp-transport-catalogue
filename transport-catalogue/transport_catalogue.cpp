#include "transport_catalogue.h"
#include "json_builder.h"

namespace transport_catalogue
{

    namespace catalogue
    {

        TransportCatalogue::Stop::Stop(std::string name_, geo::Coordinates coor_)
            : name(name_)
        {
            coordinates.lat = coor_.lat;
            coordinates.lng = coor_.lng;
        }

        TransportCatalogue::Stop::Stop(Stop&& other) noexcept
        {
            name = std::move(other.name);
            coordinates.lat = other.coordinates.lat;
            coordinates.lng = other.coordinates.lng;
        }

        TransportCatalogue::Bus::Bus(Bus&& other) noexcept
        {
            bus_name = std::move(other.bus_name);
            stops = std::move(other.stops);
            is_roundtrip = other.is_roundtrip;
        }

        size_t TransportCatalogue::StopPairHasher::operator()(const std::pair<const Stop*, const Stop*> stop_pair) const
        {
            return std::hash<const void*>{}(stop_pair.first)* (37) + std::hash<const void*>{}(stop_pair.second);
        }

        double TransportCatalogue::GetDistance(const std::pair<const Stop*, const Stop*> stop_pair) const
        {
            if (distances_.count(stop_pair) == 0)
            {
                return distances_.at(std::make_pair(stop_pair.second, stop_pair.first));
            }
            return distances_.at(stop_pair);
        }

        TransportCatalogue::TransportCatalogue(reader::JsonReader* reader)
        {
            bus_velocity_ = reader->router_settings_.velocity;
            bus_wait_time_ = reader->router_settings_.wait_time;

            for (StopQuery& stop : reader->stop_queries_)
            {
                AddStop(std::move(stop));
            }

            for (BusQuery& bus : reader->bus_queries_)
            {
                AddBus(std::move(bus));
            }
        }

        void TransportCatalogue::AddStop(StopQuery&& stop_query)
        {

            if (stopname_to_stop_.count(stop_query.stop_name) != 0)
            {
                stopname_to_stop_.at(stop_query.stop_name)->coordinates = stop_query.coordinates;
            }
            else
            {
                Stop stop;
                stop.coordinates = stop_query.coordinates;
                stop.name = stop_query.stop_name;
                
                stops_.push_back(std::move(stop));
                stopname_to_index_[stops_.back().name] = stops_.size() - 1;
                stopname_to_stop_[stops_.back().name] = &stops_.back();
            }

            for (auto& [stop_name, dist] : stop_query.stop_to_distance)
            {
                if (stopname_to_stop_.count(stop_name) != 0)
                {
                    distances_[std::make_pair(stopname_to_stop_.at(stop_query.stop_name), stopname_to_stop_.at(stop_name))] = dist;

                }
                else
                {
                    Stop stop;
                    stop.name = stop_name;
                    stops_.push_back(std::move(stop));
                    stopname_to_index_[stops_.back().name] = stops_.size() - 1;
                    stopname_to_stop_[stops_.back().name] = &stops_.back();
                    distances_[std::make_pair(stopname_to_stop_.at(stop_query.stop_name), stopname_to_stop_.at(stop_name))] = dist;
                }
            }
        }

        void TransportCatalogue::AddWaitEdgeInfo(std::string_view stop_name, std::string_view bus_name, double wait_time)
        {
            EdgeInfo edge;
            edge.bus_name = bus_name;
            edge.stop_name = stop_name;
            edge.time = wait_time;
            edge_info_.push_back(std::move(edge));
        }
		
        void TransportCatalogue::AddBusEdgeInfo(std::string_view bus_name, std::string_view from, std::string_view to, int span_count, double road_time)
        {
            EdgeInfo edge;
            edge.is_road = true;
            edge.bus_name = bus_name;
            edge.from = from;
            edge.to = to;
            edge.time = road_time;
            edge.span_count = span_count;
            edge_info_.push_back(std::move(edge));
        }

        void TransportCatalogue::AddBus(BusQuery&& bus_query)
        {
            Bus bus;
            bus.bus_name = bus_query.bus_name;
            bus.is_roundtrip = bus_query.is_roundtrip;
            buses_.push_back(std::move(bus));
            std::unordered_set<std::string> uniq_stops;

            for (const auto& name : bus_query.stops)
            {
                buses_.back().stops.push_back(stopname_to_stop_.at(name));
                stops_to_buses_[stopname_to_stop_.at(name)].insert(&buses_.back());
                uniq_stops.insert(name);
            }

            size_t end_index = 0;
            if (!bus_query.is_roundtrip)
            {
                end_index = bus_query.stops.size() / 2;
            }
            else
            {
                end_index = bus_query.stops.size() - 2;
            }

            for (size_t i = 0; i < end_index; ++i)
            {
                graph_.AddEdge({ 2 * stopname_to_index_.at(bus_query.stops[i]),
                 2 * stopname_to_index_.at(bus_query.stops[i]) + 1,
                 bus_wait_time_ });
                AddWaitEdgeInfo(stopname_to_stop_.at(bus_query.stops[i])->name, buses_.back().bus_name, bus_wait_time_);
                for (size_t j = i + 1; j <= end_index; ++j)
                {
                    double dist = 0;
                    for (size_t k = i; k < j; ++k)
                    {
                        auto stop_pair = std::make_pair(stopname_to_stop_[bus_query.stops[k]], stopname_to_stop_[bus_query.stops[k+1]]);
                        dist += GetDistance(stop_pair);
                    }
                    double road_time = dist / bus_velocity_ / 1000 * 60;

                    graph_.AddEdge({ 2 * stopname_to_index_.at(bus_query.stops[i]) + 1,
                                     2 * stopname_to_index_.at(bus_query.stops[j]),
                                     road_time });
                    AddBusEdgeInfo(buses_.back().bus_name, stopname_to_stop_.at(bus_query.stops[i])->name,
                        stopname_to_stop_.at(bus_query.stops[j])->name, static_cast<int>(j - i), road_time);
                }
            }
            graph_.AddEdge({ 2 * stopname_to_index_.at(bus_query.stops[end_index]), 
                2 * stopname_to_index_.at(bus_query.stops[end_index]) + 1, 
                bus_wait_time_ });
            AddWaitEdgeInfo(stopname_to_stop_.at(bus_query.stops[end_index])->name, buses_.back().bus_name, bus_wait_time_);
            for (size_t i = end_index; i < bus_query.stops.size(); ++i)
            {
                for (size_t j = i + 1; j < bus_query.stops.size(); ++j)
                {
                    double dist = 0;
                    for (size_t k = i; k < j; ++k)
                    {
                        auto stop_pair = std::make_pair(stopname_to_stop_[bus_query.stops[k]], stopname_to_stop_[bus_query.stops[k + 1]]);
                        dist += GetDistance(stop_pair);
                    }
                    double road_time = dist / bus_velocity_ / 1000 * 60;
                    graph_.AddEdge({ 2 * stopname_to_index_.at(bus_query.stops[i]) + 1,
                                     2 * stopname_to_index_.at(bus_query.stops[j]),
                                     road_time });
                    AddBusEdgeInfo(buses_.back().bus_name, stopname_to_stop_.at(bus_query.stops[i])->name,
                        stopname_to_stop_.at(bus_query.stops[j])->name, static_cast<int>(j - i), road_time);
                }
            }
            if (bus_query.is_roundtrip)
            {
                size_t j = bus_query.stops.size() - 1;
                for (int i = 1; i < end_index; ++i)
                {
                    double dist = 0;
                    for (size_t k = i; k < j; ++k)
                    {
                        auto stop_pair = std::make_pair(stopname_to_stop_[bus_query.stops[k]], stopname_to_stop_[bus_query.stops[k + 1]]);
                        dist += GetDistance(stop_pair);
                    }
                    double road_time = dist / bus_velocity_ / 1000 * 60;
                    graph_.AddEdge({ 2 * stopname_to_index_.at(bus_query.stops[i]) + 1,
                                     2 * stopname_to_index_.at(bus_query.stops[j]),
                                     road_time });
                    AddBusEdgeInfo(buses_.back().bus_name, stopname_to_stop_.at(bus_query.stops[i])->name,
                        stopname_to_stop_.at(bus_query.stops[j])->name, static_cast<int>(j - i), road_time);
                }
            }
            buses_.back().number_of_uniq_stops = uniq_stops.size();
            busname_to_bus_.emplace(std::make_pair(std::string_view(buses_.back().bus_name), &buses_.back()));
        }

        BusInfo TransportCatalogue::GetBusInfo(std::string_view bus_name) const
        {
            BusInfo bus_info;
            bus_info.name = bus_name;
            if (busname_to_bus_.count(bus_name) == 0)
            {
                return bus_info;
            }
            bus_info.is_found = true;
            const Bus& bus = *busname_to_bus_.at(bus_name);
            geo::Coordinates from = bus.stops[0]->coordinates;
            geo::Coordinates to;
            double geo_distance = 0;
            for (size_t i = 1; i < bus.stops.size(); ++i)
            {
                to = bus.stops[i]->coordinates;
                geo_distance += ComputeDistance(from, to);
                from = to;
            }
            double actual_distance = 0;
            const Stop* stop_from = bus.stops[0];
            const Stop* stop_to = nullptr;
            for (size_t i = 1; i < bus.stops.size(); ++i)
            {
                stop_to = bus.stops[i];
                auto stop_pair = std::make_pair(stop_from, stop_to);
                if (distances_.count(stop_pair) == 0)
                {
                    actual_distance += distances_.at(std::make_pair(stop_to, stop_from));
                }
                else
                {
                    actual_distance += distances_.at(stop_pair);
                }
                stop_from = stop_to;
            }
            double curvature = actual_distance / geo_distance;
            bus_info.actual_distance = actual_distance;
            bus_info.curvature = curvature;
            bus_info.number_of_stops = static_cast<int>(bus.stops.size());
            bus_info.number_of_uniq_stops = static_cast<int>(bus.number_of_uniq_stops);
            return bus_info;
        }

        StopInfo TransportCatalogue::GetStopInfo(std::string_view stop_name) const
        {
            if (stopname_to_stop_.count(stop_name) == 0)
            {
                return { stop_name, false, {} };

            }
            if (stops_to_buses_.count(stopname_to_stop_.at(stop_name)) == 0)
            {
                return { stop_name, true, {} };
            }

            StopInfo stop{ stop_name, true, {} };
            for (Bus* bus : stops_to_buses_.at(stopname_to_stop_.at(stop_name)))
            {
                stop.buses.insert(bus->bus_name);
            }
            return stop;
        }

        RouteInfo TransportCatalogue::GetRouteInfo(std::string_view to, std::string_view from) const
        {
            static graph::Router<double> router_(graph_);

            auto res = router_.BuildRoute(2*stopname_to_index_.at(from), 2*stopname_to_index_.at(to));
            if (!res)
            {
                return {};
            }

            auto&& [total, edges] = res.value();

            RouteInfo route_info;
            route_info.is_found = true;
            route_info.total_time = total;
            std::vector<RouteItem> items;
            items.reserve((edges.size()));

            for (size_t i = 0; i < edges.size(); ++i)
            {
                RouteItem item;
                const auto& info = edge_info_.at(edges[i]);
                item.time = info.time;
                if (!info.is_road)
                {
                    item.type = "Wait";
                    item.stop_name = info.stop_name;
                }
                else
                {
                    item.type = "Bus";
                    item.bus_name = info.bus_name;
                    item.span_count = info.span_count;
                    
                }
                items.push_back(std::move(item));
            }

            route_info.items = std::move(items);
            return route_info;
        }

        std::set<BusView, BusViewComp> TransportCatalogue::GetBusesRenderInfo() const
        {
            std::set<BusView, BusViewComp> res;

            for (const auto& bus : buses_)
            {
                BusView bus_render_info;
                bus_render_info.name_ = bus.bus_name;
                bus_render_info.is_roundtrip = bus.is_roundtrip;
                for (auto& stop : bus.stops)
                {
                    bus_render_info.stops_.emplace_back(stop->name, stop->coordinates);
                }
                res.insert(std::move(bus_render_info));
            }
            return res;
        }

        std::vector<StopView> TransportCatalogue::GetUniqueStopsInBus() const
        {
            std::vector<StopView> res;
            res.reserve(stops_.size());
            for (const auto& [key, value] : stops_to_buses_)
            {
                if (!value.empty())
                {
                    res.emplace_back(std::string_view(key->name), key->coordinates);
                }
            }
            return res;
        }

        json::Node TransportCatalogue::BusInfoAsJson(const BusInfo& bus_info, int id) const
        {
            using namespace json;
            if (!bus_info.is_found)
            {
                return json::Builder{}.StartDict().Key("request_id").Value(id).Key("error_message").Value("not found").EndDict().Build();
            }

            return json::Builder{}.StartDict().Key("curvature").Value(bus_info.curvature)
                .Key("request_id").Value(id)
                .Key("route_length").Value(bus_info.actual_distance)
                .Key("stop_count").Value(bus_info.number_of_stops)
                .Key("unique_stop_count").Value(bus_info.number_of_uniq_stops)
                .EndDict().Build();
        }

        json::Node TransportCatalogue::StopInfoAsJson(const StopInfo& stop_info, int id) const
        {
            using namespace json;
            if (!stop_info.is_found)
            {
                return json::Builder{}.StartDict().Key("request_id").Value(id).Key("error_message").Value("not found").EndDict().Build();
            }

            Array buses;
            buses.reserve(stop_info.buses.size());

            for (auto bus : stop_info.buses)
            {
                buses.push_back(std::move(bus));
            }
            return json::Builder{}.StartDict().Key("buses").Value(buses).Key("request_id").Value(id).EndDict().Build();
        }


        json::Node TransportCatalogue::RouteInfoAsJson(const RouteInfo& route_info, int id) const
        {
            using namespace json;
            if (!route_info.is_found)
            {
                Node node = Dict{ {"request_id", id}, {"error_message", std::string("not found")} };
                return node;
            }

            Array items;

            for (const auto& item : route_info.items)
            {
                json::Node entry;
                if (item.type == "Wait")
                {
                     entry = json::Builder{}.StartDict().Key("type").Value({ item.type })
                        .Key("stop_name").Value({ item.stop_name })
                        .Key("time").Value({ item.time }).EndDict().Build();
                }
                if (item.type == "Bus")
                {
                    entry = json::Builder{}.StartDict().Key("type").Value({ item.type })
                        .Key("bus").Value({ item.bus_name })
                        .Key("span_count").Value({ item.span_count })
                        .Key("time").Value({ item.time }).EndDict().Build();
                }
                items.push_back(std::move(entry));
            }
            Node node = json::Builder{}.StartDict().Key("total_time").Value(route_info.total_time)
                .Key("request_id").Value(id)
                .Key("items").Value(items).EndDict().Build();
            return node;
        }
        


        void TransportCatalogue::SetBusWaitTime(int wait_time)
        {
            bus_wait_time_ = wait_time;
        }
        void TransportCatalogue::SetBusVelocity(int velocity)
        {
            bus_velocity_ = velocity;
        }
    }

}