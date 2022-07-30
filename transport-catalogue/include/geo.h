#pragma once

#include <cmath>

namespace transport_catalogue
{

    namespace geo
    {

        struct Coordinates {
            Coordinates()
            {

            }
            Coordinates(double a, double b);
            Coordinates(const Coordinates& other);
            double lat;
            double lng;

            bool operator==(const Coordinates& other) const;
            bool operator!=(const Coordinates& other) const;
            Coordinates& operator=(const Coordinates& other);
        };

        double ComputeDistance(Coordinates from, Coordinates to);

    }

}