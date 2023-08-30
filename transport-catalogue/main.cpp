#include "json_reader.h"
#include "transport_catalogue.h"
#include "request_handler.h"

#include <iostream>

using namespace transport_catalogue;
using namespace std;

int main() {
    TransportCatalogue db;
    input::JsonReader reader(db, std::cin);
    reader.Read();
    output::JsonRequest request(db, std::cout);
    //request.Stats(reader.GetRequest());
    request.RenderMap(reader.GetMapPreset());
}
