#include "json_builder.h"
#include "json_reader.h"
#include "map_renderer.h"
#include "transport_catalogue.h"
#include "serialization.h"
#include "transport_router.h"

#include <fstream>
#include <iostream>
#include <string_view>

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

    if (mode == "make_base"sv) {
        transport_catalogue::TransportCatalogue db;
        transport_router::TransportRouter router(db);
        renderer::MapRenderer renderer;
        json::Builder build;
        json_reader::JsonReader reader(db, renderer, router, build);
        reader.Read(std::cin);
        reader.StartParse();
        std::ofstream out(reader.ReturnSerializationSettings().name_file, std::ios::binary);

        renderer::MapSettings map_settings = renderer.GetSettings();
        serialization::CatalogueSerialization(db, map_settings, router, out);
        // make base here

    } else if (mode == "process_requests"sv) {
        transport_catalogue::TransportCatalogue db;
        transport_router::TransportRouter router(db);
        renderer::MapRenderer renderer;
        json::Builder build;
        json_reader::JsonReader reader(db, renderer, router, build);
        reader.Read(std::cin);
        
        std::ifstream in(reader.ReturnSerializationSettings().name_file, std::ios::binary);
        serialization::Catalogue catalogue;
        catalogue = serialization::CatalogueDeserialization(in);
        db = catalogue.transport_catalogue;
        renderer.AddSettings(catalogue.map_settings);

        router.SetSettings(catalogue.transport_router_data.transport_router.settings);
        router.SetVexters(catalogue.transport_router_data.transport_router.vexters);
        router.SetGraph(std::move(catalogue.transport_router_data.graph));
        router.SetRouter(std::move(catalogue.transport_router_data.transport_router.routes_internal_data));

        reader.StartParse();
        
        reader.ReturnStat(std::cout);
        // process requests here
    } else {
        PrintUsage();
        return 1;
    }
}

