#include "stat_reader.h"
#include "transport_catalogue.h"

namespace transport_catalogue
{
	namespace stat_reader
	{

		void PrintBusInfo(const BusInfo& bus_info)
		{
			if (!bus_info.is_found)
			{
				std::cout << "Bus " << bus_info.name << ": not found" << std::endl;
				return;
			}
			std::cout << "Bus " << bus_info.name << ": " << bus_info.number_of_stops << " stops on route, " << bus_info.number_of_uniq_stops
					<< " unique stops, " << std::setprecision(6) << bus_info.actual_distance
					<< " route length, " << bus_info.curvature << " curvature" << std::endl;
		}

		void PrintStopInfo(const StopInfo& stop_info)
		{
			if (!stop_info.is_found)
			{
				std::cout << "Stop " << stop_info.name << ": not found" << std::endl;
				return;
			}
			if (stop_info.buses.empty())
			{
				std::cout << "Stop " << stop_info.name << ": no buses" << std::endl;
				return;
			}
			std::cout << "Stop " << stop_info.name << ": buses";
			for (std::string_view bus : stop_info.buses)
			{
				std::cout << ' ' << bus;
			}
			std::cout << std::endl;
		}

		void ProcessInfoQueries(const catalogue::TransportCatalogue& catalogue, const reader::Reader& reader)
		{
			for (const auto& query : reader.info_queries_)
			{
				if (query.is_stop)
				{
					PrintStopInfo(catalogue.GetStopInfo(query.name));
				}
				else
				{
					PrintBusInfo(catalogue.GetBusInfo(query.name));
				}
			}
		}
	}
}