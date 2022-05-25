#pragma once

#include <iostream>

#include "transport_catalogue.h"
#include "json_reader.h"

namespace transport_catalogue
{
	json::Node ProcessInfoAsJson(const catalogue::TransportCatalogue& catalogue, const reader::JsonReader& reader, const renderer::MapRenderer& renderer);
	std::set<BusView, BusViewComp> GetBusesRenderInfo(const catalogue::TransportCatalogue& catalogue);
	std::vector<StopView> GetUniqueStopsInBus(const catalogue::TransportCatalogue& catalogue);
}