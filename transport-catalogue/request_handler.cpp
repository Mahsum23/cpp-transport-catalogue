#include <sstream>

#include "request_handler.h"
#include "json_builder.h"

namespace transport_catalogue
{
    RequestHandler::RequestHandler(const catalogue::TransportCatalogue& catalogue, const reader::JsonReader& reader, const renderer::MapRenderer& renderer)
        : catalogue_(catalogue), reader_(reader), renderer_(renderer)
    {

    }
    json::Node RequestHandler::ProcessInfoAsJson() const
    {
        json::Array node;
        for (const InfoQuery& query : reader_.GetInfoQueries())
        {
            if (query.query_type_ == QueryType::MAP)
            {
                std::stringstream ss;
                renderer_.RenderMap(catalogue_, ss);
                node.push_back(json::Builder{}.StartDict().Key("map").Value(ss.str()).Key("request_id").Value(query.id_).EndDict().Build());
            }
            else if (query.query_type_ == QueryType::STOP)
            {
                node.push_back(catalogue_.StopInfoAsJson(catalogue_.GetStopInfo(query.name_), query.id_));
            }
            else if (query.query_type_ == QueryType::BUS)
            {
                node.push_back(catalogue_.BusInfoAsJson(catalogue_.GetBusInfo(query.name_), query.id_));
            }
        }
        return node;
    }

}