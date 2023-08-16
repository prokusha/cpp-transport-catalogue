#include "input_reader.h"
#include "stat_reader.h"
#include "transport_catalogue.h"

#include <iostream>

using namespace std;
using namespace transport_catalogue;

int main() {
    TransportCatalogue transport;
    input::waitlist list;
    input::Add(list);

    for (auto& stop : list.stop) {
        transport.AddStop(stop);
    }

    transport.MarkDistance();

    for (auto& bus : list.bus) {
        transport.AddBus(bus);
    }

    output::Stat(transport);
}
