#include <fstream>
#include <iostream>
#include <string_view>
#include <sstream>


#include "transport_catalogue.h"
#include "request_handler.h"
#include "svg.h"
#include "domain.h"
#include "serialization.h"

using namespace std::literals;

void PrintUsage(std::ostream& stream = std::cerr) {
    stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        PrintUsage();
        return 1;
    }

    const std::string_view mode(argv[1]);
    using namespace transport_catalogue::catalogue;
    using namespace transport_catalogue::reader;
    using namespace transport_catalogue::renderer;
    using namespace transport_catalogue;
    if (mode == "make_base"sv) {

        JsonReader reader;
        MapRenderer renderer;
        Serializer serializer;
        reader.ParseRequest(std::cin, renderer);
        std::ofstream out(reader.file_name, std::ios::binary);
        serializer.SerializeCatalogue(reader, renderer, out);

    } else if (mode == "process_requests"sv) {
        JsonReader reader;
        MapRenderer renderer;
        Serializer serializer;
        reader.ParseRequest(std::cin, renderer);
        std::ifstream in(reader.file_name, std::ios::binary);
        serializer.DeserializeCatalogue(reader, renderer, in);
        TransportCatalogue tc(&reader);
        RequestHandler request_handler(tc, reader, renderer);
        json::Print(json::Document{ request_handler.ProcessInfoAsJson() }, std::cout);


    } else {
        PrintUsage();
        return 1;
    }
}