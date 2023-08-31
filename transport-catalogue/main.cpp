#include "json_reader.h"
#include "transport_catalogue.h"

#include <iostream>

using namespace transport_catalogue;
using namespace std;

int main() {
    TransportCatalogue db;
    JsonReader reader(db);
    reader.Read(std::cin);
    reader.ReturnStat(std::cout);
}
