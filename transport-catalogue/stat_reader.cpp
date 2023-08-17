#include "stat_reader.h"
#include "input_reader.h"
#include "transport_catalogue.h"

#include <ostream>
#include <sstream>
#include <iostream>

using namespace std;

namespace transport_catalogue {
namespace output {

ostream& operator<<(ostream& out, StatBuses stat) {
    if (stat.empty) {
        out << "not found";
        return out;
    }

    out << stat.route << " stops on route, " << stat.unique << " unique stops, " << stat.length << " route length, " << stat.curvature << " curvature";

    return out;
}

ostream& operator<<(ostream& out, StatStops stat) {
    if (stat.empty) {
        out << "not found";
        return out;
    }
    if (stat.name_bus.empty()) {
        out << "no buses";
        return out;
    }

    out << "buses ";
    for (auto& bus : stat.name_bus) {
        out << bus << ' ';
    }

    return out;
}

void ReturnStats(TransportCatalogue& transport, ostream& out) {
    int count = input::ReadLineWithNumber();

    for (int it = 0; it < count; ++it) {
        string command = input::ReadLine();
        if (command[0] == 'B') {
            DisplayStatBus(transport, command, out);
        } else if (command[0] == 'S') {
            DisplayStatStop(transport, command, out);
        }
    }
}

void DisplayStatBus(TransportCatalogue& transport, string_view command, ostream& out) {
    out << command << ": " << transport.ReturnStatBus(command) << endl;
}

void DisplayStatStop(TransportCatalogue& transport, string_view command, ostream& out) {
    out << command << ": " << transport.ReturnStatStop(command) << endl;
}

} //end output
} //end transport_catalogue
