#pragma once

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <vector>
#include <unordered_map>
#include <string>
#include <set>

#include "geo.h"
#include "svg.h"

namespace transport_catalogue
{

    enum class QueryType
    {
        MAP,
        BUS,
        STOP
    };

    struct StopQuery
    {
        StopQuery() = default;
        StopQuery(StopQuery&& other) noexcept;

        std::unordered_map<std::string, double> stop_to_distance;
        std::string stop_name = "";
        geo::Coordinates coordinates{0,0};
    };

    struct BusQuery
    {
        BusQuery() = default;
        BusQuery(BusQuery&& other) noexcept;

        std::string bus_name;
        std::vector<std::string> stops;
        bool is_roundtrip = true;
    };

    struct InfoQuery
    {
        InfoQuery() = default;
        InfoQuery(int id, std::string&& name, QueryType query_type);

        int id_ = 0;
        std::string name_;
        QueryType query_type_;
    };
    struct BusInfo
    {
        int id = 0;
        std::string_view name;
        bool is_found = false;
        double actual_distance = 0;
        double curvature = 0;
        int number_of_stops = 0;
        int number_of_uniq_stops = 0;
    };

    struct StopInfo
    {
        std::string_view name;
        bool is_found = false;
        std::set<std::string> buses;
    };

    struct StopView
    {
        StopView(std::string_view stop_name, geo::Coordinates coor);
        std::string_view name_;
        geo::Coordinates coor_{ 0,0 };
    };

    struct BusView
    {
        std::string_view name_;
        std::vector<StopView> stops_;
        bool is_roundtrip = true;
    };

    struct BusViewComp
    {
        bool operator()(const BusView& lhs, const BusView& rhs) const;
    };

    inline const double EPSILON = 1e-6;
    inline bool IsZero(double value)
    {
        return std::abs(value) < EPSILON;
    }

    class SphereProjector
    {
    public:
        template <typename PointInputIt>
        SphereProjector(PointInputIt points_begin, PointInputIt points_end,
            double max_width, double max_height, double padding)
            : padding_(padding) //
        {
            if (points_begin == points_end)
            {
                return;
            }

            const auto [left_it, right_it] = std::minmax_element(
                points_begin, points_end,
                [](auto lhs, auto rhs) { return lhs.lng < rhs.lng; });
            min_lon_ = left_it->lng;
            const double max_lon = right_it->lng;

            const auto [bottom_it, top_it] = std::minmax_element(
                points_begin, points_end,
                [](auto lhs, auto rhs) { return lhs.lat < rhs.lat; });
            const double min_lat = bottom_it->lat;
            max_lat_ = top_it->lat;

            std::optional<double> width_zoom;
            if (!IsZero(max_lon - min_lon_))
            {
                width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
            }

            std::optional<double> height_zoom;
            if (!IsZero(max_lat_ - min_lat))
            {
                height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
            }

            if (width_zoom && height_zoom) 
            {
                zoom_coeff_ = std::min(*width_zoom, *height_zoom);
            }
            else if (width_zoom) 
            {
                zoom_coeff_ = *width_zoom;
            }
            else if (height_zoom) 
            {
                zoom_coeff_ = *height_zoom;
            }
        }

        svg::Point operator()(geo::Coordinates coords) const;

    private:
        double padding_;
        double min_lon_ = 0;
        double max_lat_ = 0;
        double zoom_coeff_ = 0;
    };
}
