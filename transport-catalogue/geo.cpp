#include <cmath>

#include "geo.h"

namespace transport_catalogue
{

    namespace geo
    {
        const int EARTH_RADIUS = 6371000;
        const double pi = 3.1415926535;

        Coordinates::Coordinates(double a, double b)
            : lat(a), lng(b)
        {
        }

        Coordinates::Coordinates(const Coordinates& other)
        {
            lat = other.lat;
            lng = other.lng;
        }

        bool Coordinates::operator==(const Coordinates& other) const {
            return lat == other.lat && lng == other.lng;
        }
        bool Coordinates::operator!=(const Coordinates& other) const {
            return !(*this == other);
        }
        Coordinates& Coordinates::operator=(const Coordinates& other)
        {
            lat = other.lat;
            lng = other.lng;
            return *this;
        }


        double ComputeDistance(Coordinates from, Coordinates to) {
            using namespace std;
            if (from == to) {
                return 0;
            }
            static const double dr = pi / 180.;
            return acos(sin(from.lat * dr) * sin(to.lat * dr)
                + cos(from.lat * dr) * cos(to.lat * dr) * cos(abs(from.lng - to.lng) * dr))
                * EARTH_RADIUS;
        }

    }

}