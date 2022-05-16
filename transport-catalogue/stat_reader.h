#pragma once

#include <string_view>
#include "input_reader.h"

namespace transport_catalogue
{
	struct StopInfo;
	struct BusInfo;

	namespace stat_reader
	{
		void PrintStopInfo(const StopInfo& stop_info);
		void PrintBusInfo(const BusInfo& bus_info);
		void ProcessInfoQueries(const catalogue::TransportCatalogue& catalogue, const reader::Reader& reader);
	}
}