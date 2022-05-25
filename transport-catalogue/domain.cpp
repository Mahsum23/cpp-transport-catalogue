#include "domain.h"
#include "svg.h"

namespace transport_catalogue
{
	StopQuery::StopQuery(StopQuery&& other) noexcept
	{
		stop_name = std::move(other.stop_name);
		stop_to_distance = std::move(other.stop_to_distance);
		coordinates.lat = other.coordinates.lat;
		coordinates.lng = other.coordinates.lng;
	}

	BusQuery::BusQuery(BusQuery&& other) noexcept
	{
		bus_name = std::move(other.bus_name);
		stops = std::move(other.stops);
		is_roundtrip = other.is_roundtrip;
	}

	InfoQuery::InfoQuery(int id, std::string&& name, QueryType query_type)
		: id_(id), name_(name), query_type_(query_type)
	{

	}

    svg::Point SphereProjector::operator()(geo::Coordinates coords) const {
        return {
            (coords.lng - min_lon_) * zoom_coeff_ + padding_,
            (max_lat_ - coords.lat) * zoom_coeff_ + padding_
        };
    }

}


