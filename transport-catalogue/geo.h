#pragma once

namespace transport_catalogue
{

    namespace geo
    {

        struct Coordinates 
        {
            double lat;
            double lng;
            bool operator==(const Coordinates& other) const;
            bool operator!=(const Coordinates& other) const;
            Coordinates& operator=(const Coordinates& other);
        };

        double ComputeDistance(Coordinates from, Coordinates to);

    }

}