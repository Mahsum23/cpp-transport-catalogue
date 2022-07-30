#include <transport_catalogue.pb.h>

#include "transport_catalogue.h"

namespace transport_catalogue
{
    class Serializer
    {
    public:
        void SerializeCatalogue(reader::JsonReader& jr, renderer::MapRenderer& mr, std::ostream& out);
        void DeserializeCatalogue(reader::JsonReader& jr, renderer::MapRenderer& mr, std::istream& in);

    private:
        serialize::Color MakeProtoColor(svg::Color color);
        svg::Color MakeSvgColor(serialize::Color proto_color);
        serialize::Renderer MakeProtoRenderer(renderer::MapRenderer& mr);
        void FillRendererFromProto(renderer::MapRenderer& mr, const serialize::Renderer& smr);
    };
}
