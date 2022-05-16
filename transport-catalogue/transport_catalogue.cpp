#include "transport_catalogue.h"


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
		}

		size_t TransportCatalogue::StopPairHasher::operator()(const std::pair<const Stop*, const Stop*> stop_pair) const
		{
			const size_t a = *reinterpret_cast<const size_t*>(&stop_pair.first);
			const size_t b = *reinterpret_cast<const size_t*>(&stop_pair.second);
			return a * 17 + b;
		}

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
			std::unordered_set<std::string> uniq_stops;
			for (const auto& name : bus_query.stops)
			{
				buses_.back().stops.push_back(stopname_to_stop_.at(name));
				stops_to_buses_[stopname_to_stop_.at(name)].insert(&buses_.back());
				uniq_stops.insert(name);
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
			bus_info.number_of_stops = bus.stops.size();
			bus_info.number_of_uniq_stops = bus.number_of_uniq_stops;
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

	}

}