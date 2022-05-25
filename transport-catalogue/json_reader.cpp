#include "json_reader.h"

namespace transport_catalogue
{
	namespace reader
	{

		void JsonReader::ParseRequest(std::istream& in, renderer::MapRenderer& renderer)
		{
			using namespace json;
			using namespace std::literals;
			Document doc = Load(in);
			Node root = doc.GetRoot();
			if (!root.IsMap())
			{
				throw std::invalid_argument("error in json structure");
			}
			for (const auto& [key, value] : root.AsMap())
			{
				if (key == "base_requests"s)
				{
					ParseBaseRequest(value);
				}
				else if (key == "stat_requests")
				{
					ParseStatRequest(value);
				}
				else if (key == "render_settings")
				{
					ParseRenderSettings(value, renderer);
				}
			}
		}

		void JsonReader::ParseBaseRequest(const json::Node& node)
		{
			using namespace json;
			using namespace std::literals;
			for (const Node& request : node.AsArray())
			{
				if (request.AsMap().at("type"s) == "Bus"s)
				{
					ParseBusQuery(request);
				}
				if (request.AsMap().at("type"s) == "Stop"s)
				{
					ParseStopQuery(request);
				}
			}
		}

		void JsonReader::ParseStatRequest(const json::Node& node)
		{
			using namespace json;
			using namespace std::literals;
			for (const Node& request : node.AsArray())
			{
				InfoQuery info_query;
				info_query.id_ = request.AsMap().at("id").AsInt();
				if (request.AsMap().at("type").AsString() == "Map")
				{
					info_query.id_ = request.AsMap().at("id").AsInt();
					info_query.query_type_ = QueryType::MAP;
				}
				else if (request.AsMap().at("type").AsString() == "Stop")
				{
					info_query.id_ = request.AsMap().at("id").AsInt();
					info_query.name_ = request.AsMap().at("name").AsString();
					info_query.query_type_ = QueryType::STOP;
				}
				else if (request.AsMap().at("type").AsString() == "Bus")
				{
					info_query.id_ = request.AsMap().at("id").AsInt();
					info_query.name_ = request.AsMap().at("name").AsString();
					info_query.query_type_ = QueryType::BUS;
				}
				else
				{
					throw std::invalid_argument("Unknown query");
				}
				info_queries_.push_back(std::move(info_query));
			}
		}

		svg::Color NodeToColor(const json::Node& node)
		{
			if (node.IsArray())
			{
				const auto& color_array = node.AsArray();
				if (color_array.size() == 3)
				{
					return svg::Rgb{ color_array[0].AsInt(), color_array[1].AsInt(), color_array[2].AsInt() };
				}
				else if (color_array.size() == 4)
				{
					return svg::Rgba{ color_array[0].AsInt(), color_array[1].AsInt(), color_array[2].AsInt(), color_array[3].AsDouble() };
				}
			}
			return node.AsString();
		}

		void JsonReader::ParseRenderSettings(const json::Node & node, renderer::MapRenderer& renderer)
		{
			renderer::MapRenderer& mr = renderer;
			const auto& settings = node.AsMap();
			mr.SetWidth(settings.at("width").AsDouble());
			mr.SetHeight(settings.at("height").AsDouble());
			mr.SetPadding(settings.at("padding").AsDouble());
			mr.SetLineWidth(settings.at("line_width").AsDouble());
			mr.SetStopRadius(settings.at("stop_radius").AsDouble());
			mr.SetBusLabelFontSize(settings.at("bus_label_font_size").AsInt());
			mr.SetBusLabelOffset({ settings.at("bus_label_offset").AsArray()[0].AsDouble(), settings.at("bus_label_offset").AsArray()[1].AsDouble() });
			mr.SetStopLabelFontSize(settings.at("stop_label_font_size").AsInt());
			mr.SetStopLabelOffset({ settings.at("stop_label_offset").AsArray()[0].AsDouble(), settings.at("stop_label_offset").AsArray()[1].AsDouble() });
			mr.SetUnderlayerColor(NodeToColor(settings.at("underlayer_color")));
			mr.SetUnderlayerWidth(settings.at("underlayer_width").AsDouble());
			std::vector<svg::Color> palette;
			for (const auto& node : settings.at("color_palette").AsArray())
			{
				palette.push_back(NodeToColor(node));
			}
			mr.SetColorPalette(std::move(palette));
		}

		void JsonReader::ParseBusQuery(const json::Node& query)
		{
			using namespace std::literals;
			BusQuery bus;
			bus.bus_name = query.AsMap().at("name"s).AsString();
			bus.stops.reserve(query.AsMap().at("stops"s).AsArray().size());
			for (const json::Node& stop : query.AsMap().at("stops"s).AsArray())
			{
				bus.stops.push_back(stop.AsString()); // not moving
			}
			if (!query.AsMap().at("is_roundtrip"s).AsBool())
			{
				bus.stops.reserve(2 * bus.stops.size());
				bus.is_roundtrip = false;
				bus.stops.insert(bus.stops.end(), bus.stops.rbegin() + 1, bus.stops.rend());
			}
			bus_queries_.push_back(std::move(bus));
		}

		void JsonReader::ParseStopQuery(const json::Node& query)
		{
			using namespace std::literals;
			StopQuery stop_query;
			stop_query.stop_name = query.AsMap().at("name"s).AsString();
			stop_query.coordinates.lat = query.AsMap().at("latitude"s).AsDouble();
			stop_query.coordinates.lng = query.AsMap().at("longitude"s).AsDouble();
			for (const auto& [stop, dist] : query.AsMap().at("road_distances").AsMap()) // not moving
			{
				stop_query.stop_to_distance[stop] = dist.AsDouble();
			}
			stop_queries_.push_back(std::move(stop_query));
		}

		const std::vector<InfoQuery>& JsonReader::GetInfoQueries() const
		{
			return info_queries_;
		}
	}
}