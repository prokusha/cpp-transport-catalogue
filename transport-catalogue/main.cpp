#include "json_reader.h"
#include "map_renderer.h"
#include "transport_catalogue.h"

#include <iostream>

using namespace transport_catalogue;
using namespace json_reader;
using namespace std;

int main() {
    TransportCatalogue db;
    renderer::MapRenderer renderer;
    JsonReader reader(db, renderer);
    reader.Read(std::cin);
    reader.ReturnStat(std::cout);
    //reader.ReturnMap(std::cout);
}
