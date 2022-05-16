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

	struct BusInfo
	{
		std::string_view name;
		bool is_found = false;
		double actual_distance = 0;
		double curvature = 0;
		size_t number_of_stops = 0;
		size_t number_of_uniq_stops = 0;
	};

	struct StopInfo
	{
		std::string_view name;
		bool is_found = false;
		std::set<std::string> buses;
	};

	namespace catalogue
	{

		class TransportCatalogue
		{

		public:
			TransportCatalogue(reader::Reader& reader);

			void AddStop(reader::StopQuery&& stop);
			void AddBus(reader::BusQuery&& bus);
			BusInfo GetBusInfo(std::string_view bus_name) const;
			StopInfo GetStopInfo(std::string_view stop_name) const;

		private:
			struct Stop
			{
				Stop() = default;
				Stop(std::string name_, geo::Coordinates coor_);
				Stop(Stop&& other) noexcept;

				std::string name;
				geo::Coordinates coordinates;
			};

			struct StopPairHasher
			{
				size_t operator()(const std::pair<const Stop*, const Stop*> stop_pair) const;

			private:
				std::hash<const void*> hasher_;
			};



			struct Bus
			{
				Bus() = default;
				Bus(Bus&& other) noexcept;

				std::string bus_name;
				std::vector<Stop*> stops;
				size_t number_of_uniq_stops = 0;
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
