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
#include "json_reader.h"
#include "domain.h"

namespace transport_catalogue
{
	class RequestHandler;
	namespace catalogue
	{
		class TransportCatalogue;
	}
	struct BusView;
	struct BusViewComp;
	struct StopView;

	namespace catalogue
	{

		class TransportCatalogue
		{
			friend class transport_catalogue::RequestHandler;
			
		public:
			
			struct Stop
			{
				Stop() = default;
				Stop(std::string name_, geo::Coordinates coor_);
				Stop(Stop&& other) noexcept;

				std::string name;
				geo::Coordinates coordinates{ 0,0 };
			};

			struct Bus
			{
				Bus() = default;
				Bus(Bus&& other) noexcept;

				std::string bus_name;
				std::vector<Stop*> stops;
				bool is_roundtrip = true;
				size_t number_of_uniq_stops = 0;
			};

			TransportCatalogue(reader::JsonReader* reader);

			void AddStop(StopQuery&& stop);
			void AddBus(BusQuery&& bus);
			BusInfo GetBusInfo(std::string_view bus_name) const;
			StopInfo GetStopInfo(std::string_view stop_name) const;
			json::Node StopInfoAsJson(const StopInfo& stop_info, int id) const;
			json::Node BusInfoAsJson(const BusInfo& bus_info, int id) const;
			std::set<BusView, BusViewComp> GetBusesRenderInfo() const;
			std::vector<StopView> GetUniqueStopsInBus() const;
			
		private:

			struct StopPairHasher
			{
				size_t operator()(const std::pair<const Stop*, const Stop*> stop_pair) const;

			private:
				std::hash<const void*> hasher_;
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
