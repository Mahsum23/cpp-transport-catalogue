#include "request_handler.h"
#include <sstream>
namespace transport_catalogue
{

	json::Node ProcessInfoAsJson(const catalogue::TransportCatalogue& catalogue, const reader::JsonReader& reader, const renderer::MapRenderer& renderer)
	{
		json::Array node;
		for (const InfoQuery& query : reader.GetInfoQueries())
		{
			if (query.query_type_ == QueryType::MAP)
			{
				std::stringstream ss;
				renderer.RenderMap(catalogue, ss);
				node.push_back(json::Dict{ {"map", ss.str()}, {"request_id", query.id_} });
			}
			else if (query.query_type_ == QueryType::STOP)
			{
				node.push_back(catalogue.StopInfoAsJson(catalogue.GetStopInfo(query.name_), query.id_));
			}
			else if (query.query_type_ == QueryType::BUS)
			{
				node.push_back(catalogue.BusInfoAsJson(catalogue.GetBusInfo(query.name_), query.id_));
			}
		}
		return node;
	}

	std::vector<StopView> GetUniqueStopsInBus(const catalogue::TransportCatalogue& catalogue)
	{
		std::vector<StopView> res;
		res.reserve(catalogue.stops_.size());
		for (const auto& [key, value] : catalogue.stops_to_buses_)
		{
			if (!value.empty())
			{
				res.emplace_back(std::string_view(key->name), key->coordinates);
			}
		}
		return res;
	}

	std::set<BusView, BusViewComp> GetBusesRenderInfo(const catalogue::TransportCatalogue& catalogue)
	{
		std::set<BusView, BusViewComp> res;
		for (const auto& bus : catalogue.buses_)
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

}