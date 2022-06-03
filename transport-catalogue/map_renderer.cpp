#include <unordered_set>

#include "map_renderer.h"
#include "transport_catalogue.h"

namespace transport_catalogue
{

    namespace renderer
    {

        void MapRenderer::SetWidth(double width)
        {
            width_ = width;
        }
        void MapRenderer::SetHeight(double height)
        {
            height_ = height;
        }
        void MapRenderer::SetPadding(double padding)
        {
            padding_ = padding;
        }
        void MapRenderer::SetLineWidth(double line_width)
        {
            line_width_ = line_width;
        }
        void MapRenderer::SetStopRadius(double stop_radius)
        {
            stop_radius_ = stop_radius;
        }
        void MapRenderer::SetBusLabelFontSize(int bus_label_font_size)
        {
            bus_label_font_size_ = bus_label_font_size;
        }
        void MapRenderer::SetBusLabelOffset(svg::Point bus_label_offset)
        {
            bus_label_offset_ = bus_label_offset;
        }
        void MapRenderer::SetStopLabelFontSize(int stop_label_font_size)
        {
            stop_label_font_size_ = stop_label_font_size;
        }
        void MapRenderer::SetStopLabelOffset(svg::Point stop_label_offset)
        {
            stop_label_offset_ = stop_label_offset;
        }
        void MapRenderer::SetUnderlayerColor(svg::Color underlayer_color)
        {
            underlayer_color_ = underlayer_color;
        }
        void MapRenderer::SetUnderlayerWidth(double underlayer_width)
        {
            underlayer_width_ = underlayer_width;
        }
        void MapRenderer::SetColorPalette(std::vector<svg::Color>&& color_palette)
        {
            color_palette_ = std::move(color_palette);
        }

        void MapRenderer::AddBusLines(svg::Document& doc, const std::set<BusView, BusViewComp>& buses, const SphereProjector& proj) const
        {
            int palette_counter = 0;
            for (auto& bus : buses)
            {
                svg::Polyline bus_line;
                bus_line.SetFillColor("none");
                bus_line.SetStrokeColor(color_palette_[palette_counter]);
                if (!bus.stops_.empty())
                {
                    ++palette_counter;
                    palette_counter = palette_counter % color_palette_.size();
                }
                bus_line.SetStrokeWidth(line_width_);
                bus_line.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
                bus_line.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);


                for (auto& stop : bus.stops_)
                {
                    bus_line.AddPoint(proj(stop.coor_));
                }
                doc.Add(bus_line);
            }
        }

        void MapRenderer::AddBusTextLabels(svg::Document& doc, const std::set<BusView, BusViewComp>& buses, const SphereProjector& proj) const
        {
            int palette_counter = 0;
            for (auto& bus : buses)
            {
                if (bus.stops_.empty())
                    continue;

                svg::Text label1;
                svg::Text back1;
                label1.SetPosition(proj(bus.stops_[0].coor_))
                    .SetOffset(bus_label_offset_)
                    .SetData(static_cast<std::string>(bus.name_))
                    .SetFontSize(bus_label_font_size_)
                    .SetFontFamily("Verdana")
                    .SetFontWeight("bold")
                    .SetFillColor(color_palette_[palette_counter]);
    
                back1.SetPosition(proj(bus.stops_[0].coor_))
                    .SetOffset(bus_label_offset_)
                    .SetData(static_cast<std::string>(bus.name_))
                    .SetFontSize(bus_label_font_size_)
                    .SetFontFamily("Verdana")
                    .SetFontWeight("bold")
                    .SetFillColor(underlayer_color_)
                    .SetStrokeColor(underlayer_color_)
                    .SetStrokeWidth(underlayer_width_)
                    .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                    .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);


                doc.Add(back1);
                doc.Add(label1);
                if (bus.is_roundtrip || (bus.stops_[0].name_ == bus.stops_[bus.stops_.size() / 2].name_))
                {
                    ++palette_counter;
                    palette_counter = palette_counter % color_palette_.size();
                    continue;
                }
                svg::Text label2;
                svg::Text back2;
                label2.SetPosition(proj(bus.stops_[bus.stops_.size() / 2].coor_))
                    .SetOffset(bus_label_offset_)
                    .SetData(static_cast<std::string>(bus.name_))
                    .SetFontSize(bus_label_font_size_)
                    .SetFontFamily("Verdana")
                    .SetFontWeight("bold")
                    .SetFillColor(color_palette_[palette_counter]);

                back2.SetPosition(proj(bus.stops_[bus.stops_.size() / 2].coor_))
                    .SetOffset(bus_label_offset_)
                    .SetData(static_cast<std::string>(bus.name_))
                    .SetFontSize(bus_label_font_size_)
                    .SetFontFamily("Verdana")
                    .SetFontWeight("bold")
                    .SetFillColor(underlayer_color_)
                    .SetStrokeColor(underlayer_color_)
                    .SetStrokeWidth(underlayer_width_)
                    .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                    .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

                ++palette_counter;
                palette_counter = palette_counter % color_palette_.size();

                doc.Add(back2);
                doc.Add(label2);
                
            }
        }

        void MapRenderer::AddStopCircles(svg::Document& doc, std::vector<StopView>& stops, const SphereProjector& proj) const
        {
            std::sort(stops.begin(), stops.end(), [](const auto& lhs, const auto& rhs)
                {
                    return lhs.name_ < rhs.name_;
                });
            for (const auto& stop : stops)
            {
                svg::Circle stop_circle;
                stop_circle.SetCenter(proj(stop.coor_));
                stop_circle.SetFillColor("white");
                stop_circle.SetRadius(stop_radius_);
                doc.Add(stop_circle);
            }
        }

        svg::Text MapRenderer::GetStopLabelBack(svg::Point point, std::string&& name) const
        {
            svg::Text res;
            res.SetData(std::move(name))
                .SetPosition(point)
                .SetOffset(stop_label_offset_)
                .SetFontSize(stop_label_font_size_)
                .SetFontFamily("Verdana")
                .SetFillColor(underlayer_color_)
                .SetStrokeColor(underlayer_color_)
                .SetStrokeWidth(underlayer_width_)
                .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
            return res;
        }

        svg::Text MapRenderer::GetStopLabel(svg::Point point, std::string&& name) const
        {
            svg::Text res;
            res.SetData(std::move(name))
                .SetPosition(point)
                .SetOffset(stop_label_offset_)
                .SetFontSize(stop_label_font_size_)
                .SetFontFamily("Verdana")
                .SetFillColor("black");
            return res;
        }

        void MapRenderer::AddStopLabels(svg::Document& doc, std::vector<StopView>& stops, const SphereProjector& proj) const
        {
            for (const auto& stop : stops)
            {
                doc.Add(GetStopLabelBack(proj(stop.coor_), static_cast<std::string>(stop.name_)));
                doc.Add(GetStopLabel(proj(stop.coor_), static_cast<std::string>(stop.name_)));
            }
        }

        void MapRenderer::RenderMap(const catalogue::TransportCatalogue& tc, std::ostream& out) const
        {
            std::set<BusView, BusViewComp> bus_renders = tc.GetBusesRenderInfo();
            std::vector<geo::Coordinates> coordinates;
            svg::Document doc;
            
            std::vector<StopView> uniq_stops_in_bus = tc.GetUniqueStopsInBus();
            coordinates.reserve(uniq_stops_in_bus.size());
            std::transform(uniq_stops_in_bus.begin(), uniq_stops_in_bus.end(), std::back_inserter(coordinates),
                [](const auto& stop) { return stop.coor_; });

            const SphereProjector proj{ coordinates.begin(), coordinates.end(), width_, height_, padding_ };
            AddBusLines(doc, bus_renders, proj);
            AddBusTextLabels(doc, bus_renders, proj);
            AddStopCircles(doc, uniq_stops_in_bus, proj);
            AddStopLabels(doc, uniq_stops_in_bus, proj);
            doc.Render(out);
        }

    } // render

} // transport_catalogue