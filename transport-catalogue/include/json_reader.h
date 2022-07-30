#pragma once

#include <iostream>
#include <string>
#include <string_view>
#include <vector>
#include <set>
#include <stdexcept>
#include <memory>

#include "json.h"
#include "domain.h"
#include "map_renderer.h"



namespace transport_catalogue
{
    class Serializer;
    namespace catalogue
    {
        class TransportCatalogue;
    }
    
    namespace reader
    {

        class JsonReader
        {
        public:
            friend class catalogue::TransportCatalogue;
            friend class transport_catalogue::Serializer;
            void ParseRequest(std::istream& in, renderer::MapRenderer& renderer);
            void ParseBaseRequest(const json::Node& node);
            void ParseStatRequest(const json::Node& node);
            void ParseRenderSettings(const json::Node& node, renderer::MapRenderer& renderer);
            void ParseRouterSettings(const json::Node& node);
            void ParseSerializationSettings(const json::Node& node);
            const std::vector<InfoQuery>& GetInfoQueries() const;
            std::string file_name;
            

        private:
            void ParseBusQuery(const json::Node& query);
            void ParseStopQuery(const json::Node& query);
            std::vector<StopQuery> stop_queries_;
            std::vector<BusQuery> bus_queries_;
            std::vector<InfoQuery> info_queries_;
            RouterSettings router_settings_;
            
            
        };
    }

}