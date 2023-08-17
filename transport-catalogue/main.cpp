#include "input_reader.h"
#include "stat_reader.h"
#include "transport_catalogue.h"

#include <iostream>

using namespace std;
using namespace transport_catalogue;

int main() {
    TransportCatalogue transport;
    input::Add(transport, std::cin);
    output::ReturnStats(transport, std::cin, std::cout);
}
