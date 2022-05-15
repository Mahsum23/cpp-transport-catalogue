#include "transport_catalogue.h"


namespace transport_catalogue
{

	namespace catalogue
	{
		class reader::Reader;
		TransportCatalogue::TransportCatalogue(reader::Reader& reader)
		{
			for (reader::StopQuery& stop : reader.stop_queries_)
			{
				AddStop(std::move(stop));
			}
			for (reader::BusQuery& bus : reader.bus_queries_)
			{
				AddBus(std::move(bus));
			}
		}

		void TransportCatalogue::AddStop(reader::StopQuery&& stop_query)
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
					stopname_to_stop_[stops_.back().name] = &stops_.back();
					distances_[std::make_pair(stopname_to_stop_.at(stop_query.stop_name), stopname_to_stop_.at(stop_name))] = dist;
				}
			}
		}

		void TransportCatalogue::AddBus(reader::BusQuery&& bus_query)
		{
			Bus bus;
			bus.bus_name = bus_query.bus_name;
			buses_.push_back(std::move(bus));
			for (const auto& name : bus_query.stops)
			{
				buses_.back().stops.push_back(stopname_to_stop_.at(name));
				stops_to_buses_[stopname_to_stop_.at(name)].insert(&buses_.back());
			}
			busname_to_bus_.emplace(std::make_pair(std::string_view(buses_.back().bus_name), &buses_.back()));
		}

		void TransportCatalogue::GetBusInfo(std::string_view bus_name)
		{
			if (busname_to_bus_.count(bus_name) == 0)
			{
				std::cout << "Bus " << bus_name << ": not found" << std::endl;
				return;
			}
			const Bus& bus = *busname_to_bus_[bus_name];
			std::unordered_set<Stop*> uniq_stops;
			for (Stop* stop : bus.stops)
			{
				uniq_stops.insert(stop);
			}
			size_t unique_stops_count = uniq_stops.size();
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
			std::cout << "Bus " << bus_name << ": " << bus.stops.size() << " stops on route, " << unique_stops_count
				<< " unique stops, " << std::setprecision(6) << actual_distance
				<< " route length, " << curvature << " curvature" << std::endl;
		}

		void TransportCatalogue::GetStopInfo(std::string_view stop_name)
		{
			if (stopname_to_stop_.count(stop_name) == 0)
			{
				std::cout << "Stop " << stop_name << ": not found" << std::endl;
				return;
			}
			if (stops_to_buses_.count(stopname_to_stop_.at(stop_name)) == 0)
			{
				std::cout << "Stop " << stop_name << ": no buses" << std::endl;
				return;
			}
			std::cout << "Stop " << stop_name << ": buses";
			std::set<std::string_view> bus_names;
			for (Bus* bus : stops_to_buses_.at(stopname_to_stop_.at(stop_name)))
			{
				bus_names.insert(bus->bus_name);
				//std::cout << ' ' << bus->bus_name;
			}
			for (std::string_view name : bus_names)
			{
				std::cout << ' ' << name;
			}
			std::cout << std::endl;
		}

		void TransportCatalogue::ProcessInfoQueries(const reader::Reader& reader)
		{
			for (const auto& query : reader.info_queries_)
			{
				if (query.is_stop)
				{
					GetStopInfo(query.name);
				}
				else
				{
					GetBusInfo(query.name);
				}
			}
		}

	}

}