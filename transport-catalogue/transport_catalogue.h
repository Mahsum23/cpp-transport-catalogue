#pragma once

#include <string>
#include <string_view>
#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <vector>
#include <deque>
#include <algorithm>
#include <cassert>

#include "geo.h"
#include "input_reader.h"

namespace transport_catalogue
{

	namespace catalogue
	{
		class reader::Reader;

		class TransportCatalogue
		{

		public:
			TransportCatalogue(reader::Reader& reader);

			void AddStop(reader::StopQuery&& stop);
			void AddBus(reader::BusQuery&& bus);
			void GetBusInfo(std::string_view bus_name);
			void GetStopInfo(std::string_view stop_name);
			void ProcessInfoQueries(const reader::Reader& reader);

		private:
			struct Stop
			{
				Stop() = default;
				Stop(std::string name_, geo::Coordinates coor_)
					: name(name_)
				{
					coordinates.lat = coor_.lat;
					coordinates.lng = coor_.lng;
				}
				Stop(Stop&& other) noexcept
				{
					name = std::move(other.name);
					coordinates.lat = other.coordinates.lat;
					coordinates.lng = other.coordinates.lng;
				}
				std::string name;
				geo::Coordinates coordinates;
			};

			struct StopPairHasher
			{
				size_t operator()(const std::pair<const Stop*, const Stop*> stop_pair) const
				{
					const size_t a = *reinterpret_cast<const size_t*>(&stop_pair.first);
					const size_t b = *reinterpret_cast<const size_t*>(&stop_pair.second);
					return a * 17 + b;
				}

			private:
				std::hash<const void*> hasher_;
			};



			struct Bus
			{
				Bus() = default;
				Bus(Bus&& other) noexcept
				{
					bus_name = std::move(other.bus_name);
					stops = std::move(other.stops);
				}
				std::string bus_name;
				std::vector<Stop*> stops;
			};

			std::unordered_set<std::string> stop_names_;
			std::deque<Stop> stops_;
			std::deque<Bus> buses_;
			std::unordered_map<std::string_view, Stop*> stopname_to_stop_;
			std::unordered_map<std::string_view, Bus*> busname_to_bus_;
			std::unordered_map<Stop*, std::unordered_set<Bus*>> stops_to_buses_;
			std::unordered_map<std::pair<const Stop*, const Stop*>, double, StopPairHasher> distances_;
		};

	}

}

