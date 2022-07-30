#include <unordered_map>
//#include <transport_catalogue.pb.h>

#include "serialization.h"

namespace transport_catalogue
{
    void Serializer::SerializeCatalogue(reader::JsonReader& jr, renderer::MapRenderer& mr, std::ostream& out)
    {
        serialize::Reader sjr;
        std::unordered_map<std::string, size_t> stop_order;
        size_t ind = 0;
        for (const auto& stop : jr.stop_queries_)
        {
            serialize::Stop stop_ser;
            stop_ser.set_name(stop.stop_name);
            serialize::Coordinates coor;
            coor.set_lat(stop.coordinates.lat);
            coor.set_lng(stop.coordinates.lng);
            *stop_ser.mutable_coordinates() = coor;
            for (const auto& [stop_name, dist] : stop.stop_to_distance)
            {
                stop_ser.add_distances_to_stops(dist);
                
                stop_ser.add_stops(stop_name);
            }
            stop_order[stop.stop_name] = ind;
            *sjr.add_stops() = stop_ser;
            ++ind;
        }
        for (const auto& bus : jr.bus_queries_)
        {
            serialize::Bus bus_ser;
            bus_ser.set_name(bus.bus_name);
            bus_ser.set_is_roundtrip(bus.is_roundtrip);
            for (const auto& stop : bus.stops)
            {
                bus_ser.add_stops(stop_order[stop]);
            }
            *sjr.add_buses() = bus_ser;
        }
        sjr.set_bus_velocity(jr.router_settings_.velocity);
        sjr.set_wait_time(jr.router_settings_.wait_time);
        *sjr.mutable_renderer() = MakeProtoRenderer(mr);
        sjr.SerializeToOstream(&out);
    }
    void Serializer::DeserializeCatalogue(reader::JsonReader& jr, renderer::MapRenderer& mr, std::istream& in)
    {
        serialize::Reader sjr;
        sjr.ParseFromIstream(&in);
        for (const auto& stop_ser : sjr.stops())
        {
            transport_catalogue::StopQuery sq;
            sq.stop_name = stop_ser.name();
            sq.coordinates = geo::Coordinates{ stop_ser.coordinates().lat(), stop_ser.coordinates().lng() };
            std::unordered_map<std::string, double> stop_to_dist;
            for (int i = 0; i < stop_ser.stops_size(); ++i)
            {
                stop_to_dist.insert(std::make_pair(stop_ser.stops()[i], stop_ser.distances_to_stops()[i]));
            }
            sq.stop_to_distance = std::move(stop_to_dist);
            jr.stop_queries_.push_back(std::move(sq));
        }
        for (const auto& bus_ser : sjr.buses())
        {
            transport_catalogue::BusQuery bq;
            bq.bus_name = bus_ser.name();
            bq.is_roundtrip = bus_ser.is_roundtrip();
            for (size_t stop_num : bus_ser.stops())
            {
                bq.stops.push_back(jr.stop_queries_[stop_num].stop_name);
            }
            jr.bus_queries_.push_back(std::move(bq));
        }

        jr.router_settings_.velocity = sjr.bus_velocity();
        jr.router_settings_.wait_time = sjr.wait_time();
        FillRendererFromProto(mr, *sjr.mutable_renderer());
    }

    serialize::Color Serializer::MakeProtoColor(svg::Color color)
    {
        serialize::Color proto_color;

        if (std::holds_alternative<std::string>(color))
        {
            proto_color.set_str_color(std::get<std::string>(color));
        }
        else if (std::holds_alternative<svg::Rgb>(color))
        {
            serialize::Rgb rgb;
            rgb.set_blue(std::get<svg::Rgb>(color).blue);
            rgb.set_green(std::get<svg::Rgb>(color).green);
            rgb.set_red(std::get<svg::Rgb>(color).red);
            *proto_color.mutable_rgb_color() = rgb;
        }
        else
        {
            serialize::Rgba rgba;
            rgba.set_blue(std::get<svg::Rgba>(color).blue);
            rgba.set_green(std::get<svg::Rgba>(color).green);
            rgba.set_red(std::get<svg::Rgba>(color).red);
            rgba.set_opacity(std::get<svg::Rgba>(color).opacity);
            *proto_color.mutable_rgba_color() = rgba;
        }
        return std::move(proto_color);
    }

    svg::Color Serializer::MakeSvgColor(serialize::Color proto_color)
    {
        if (proto_color.has_rgba_color())
        {
            return svg::Rgba{ proto_color.rgba_color().red(), proto_color.rgba_color().green(), proto_color.rgba_color().blue(), proto_color.rgba_color().opacity() };
        }
        else if (proto_color.has_rgb_color())
        {
            return svg::Rgb{ proto_color.rgb_color().red(), proto_color.rgb_color().green(), proto_color.rgb_color().blue() };
        }
        return proto_color.str_color();
    }

    serialize::Renderer Serializer::MakeProtoRenderer(renderer::MapRenderer& mr)
    {
        serialize::Renderer smr;
        smr.set_width(mr.width_);
        smr.set_height(mr.height_);
        smr.set_padding(mr.padding_);
        smr.set_line_width(mr.line_width_);
        smr.set_stop_radius(mr.stop_radius_);
        smr.set_bus_label_font_size(mr.bus_label_font_size_);
        smr.set_stop_label_font_size(mr.stop_label_font_size_);
        smr.set_underlayer_width(mr.underlayer_width_);

        serialize::Point blo;
        blo.set_x(mr.bus_label_offset_.x);
        blo.set_y(mr.bus_label_offset_.y);
        *smr.mutable_bus_label_offset() = blo;

        serialize::Point slo;
        slo.set_x(mr.stop_label_offset_.x);
        slo.set_y(mr.stop_label_offset_.y);
        *smr.mutable_stop_label_offset() = slo;
        
        serialize::Color underlayer_color = MakeProtoColor(mr.underlayer_color_);
        *smr.mutable_underlayer_color() = underlayer_color;
        for (auto& color : mr.color_palette_)
        {
            *smr.add_color_palette() = MakeProtoColor(color);
        }
        return std::move(smr);
    }
     

    void Serializer::FillRendererFromProto(renderer::MapRenderer& mr, const serialize::Renderer& smr)
    {
        mr.width_ = smr.width();
        mr.height_ = smr.height();
        mr.padding_ = smr.padding();
        mr.line_width_ = smr.line_width();
        mr.stop_radius_ = smr.stop_radius();
        mr.bus_label_font_size_ = smr.bus_label_font_size();
        mr.stop_label_font_size_ = smr.stop_label_font_size();
        mr.underlayer_width_ = smr.underlayer_width();
        mr.bus_label_offset_ = svg::Point{ smr.bus_label_offset().x(), smr.bus_label_offset().y() };
        mr.stop_label_offset_ = svg::Point{ smr.stop_label_offset().x(), smr.stop_label_offset().y() };
        mr.underlayer_color_ = MakeSvgColor(smr.underlayer_color());

        for (auto& proto_color : smr.color_palette())
        {
            mr.color_palette_.emplace_back(MakeSvgColor(proto_color));
        }
    }

}