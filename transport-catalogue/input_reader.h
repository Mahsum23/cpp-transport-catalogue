#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <iomanip>
#include <functional>

#include "geo.h"

namespace transport_catalogue
{

	namespace catalogue
	{
		class TransportCatalogue;
	}

	namespace util
	{
		// trim from start
		static inline std::string& Ltrim(std::string& s) {
			size_t pos = s.find_first_not_of(" \t");
			s.erase(s.begin(), s.begin() + pos);
			return s;
		}

		// trim from end
		static inline std::string& Rtrim(std::string& s) {
			size_t pos = s.find_last_not_of(" \t");
			s.erase(s.begin() + pos + 1, s.end());
			return s;
		}

		// trim from both ends
		static inline std::string& Trim(std::string& s) {
			return Ltrim(Rtrim(s));
		}
	}

	namespace reader
	{

		struct StopQuery
		{
			StopQuery() = default;
			StopQuery(StopQuery&& other) noexcept
			{
				stop_name = std::move(other.stop_name);
				stop_to_distance = std::move(other.stop_to_distance);
				coordinates.lat = other.coordinates.lat;
				coordinates.lng = other.coordinates.lng;
			}
			std::unordered_map<std::string, double> stop_to_distance;
			std::string stop_name = "";
			geo::Coordinates coordinates;
		};

		struct BusQuery
		{
			BusQuery() = default;
			BusQuery(BusQuery&& other) noexcept
			{
				bus_name = std::move(other.bus_name);
				stops = std::move(other.stops);
			}
			std::string bus_name;
			std::vector<std::string> stops;
		};

		struct InfoQuery
		{
			InfoQuery(std::string&& name_, bool is_stop_)
				: name(name_), is_stop(is_stop_)
			{

			}
			std::string name;
			bool is_stop;
		};




		class Reader
		{
		public:
			void ReadQuery(std::istream& in);

			void PrintStopQueries();

			void PrintBusQueries();

		private:
			friend class catalogue::TransportCatalogue;

			std::pair<std::string, double> ParseDistToStop(const std::string& query);

			std::vector<StopQuery> stop_queries_;
			std::vector<BusQuery> bus_queries_;
			std::vector<InfoQuery> info_queries_;
		};

	}

}
