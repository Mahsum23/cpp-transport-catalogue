#pragma once

#include <deque>

#include "svg.h"
#include "domain.h"


namespace transport_catalogue
{
    class Serializer;
    namespace catalogue
    {
        class TransportCatalogue;
    }

    namespace renderer
    {

        class MapRenderer
        {
            friend class transport_catalogue::Serializer;
        public:
            void SetWidth(double width);
            void SetHeight(double height);
            void SetPadding(double padding);
            void SetLineWidth(double line_width);
            void SetStopRadius(double stop_radius);
            void SetBusLabelFontSize(int bus_label_font_size);
            void SetBusLabelOffset(svg::Point bus_label_offset);
            void SetStopLabelFontSize(int stop_label_font_size);
            void SetStopLabelOffset(svg::Point stop_label_offset);
            void SetUnderlayerColor(svg::Color underlayer_color);
            void SetUnderlayerWidth(double underlayer_width);
            void SetColorPalette(std::vector<svg::Color>&& color_palette);

            void RenderMap(const catalogue::TransportCatalogue& tc, std::ostream& out) const;
            void AddBusLines(svg::Document& doc, const std::set<BusView, BusViewComp>& buses, const SphereProjector& proj) const;
            void AddBusTextLabels(svg::Document& doc, const std::set<BusView, BusViewComp>& buses, const SphereProjector& proj) const;
            void AddStopCircles(svg::Document& doc, std::vector<StopView>& stops, const SphereProjector& proj) const;
            void AddStopLabels(svg::Document& doc, std::vector<StopView>& stops, const SphereProjector& proj) const;
            svg::Text GetStopLabelBack(svg::Point point, std::string&& name) const;
            svg::Text GetStopLabel(svg::Point point, std::string&& name) const;

        private:
            double width_ = 0;
            double height_ = 0;
            double padding_ = 0;
            double line_width_ = 0;
            double stop_radius_ = 0;
            int bus_label_font_size_ = 0;
            svg::Point bus_label_offset_{ 0,0 };
            int stop_label_font_size_ = 0;
            svg::Point stop_label_offset_{ 0,0 };
            svg::Color underlayer_color_;
            double underlayer_width_ = 0;
            std::vector<svg::Color> color_palette_;
            
        };

    }
}
