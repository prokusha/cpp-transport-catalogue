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
    if (stat.empty_bus) {
        out << "no buses";
        return out;
    }

    out << "buses ";
    for (auto& bus : stat.name_bus) {
        out << bus << ' ';
    }

    return out;
}

void Stat(TransportCatalogue& transport) {
    int count = input::ReadLineWithNumber();

    for (int it = 0; it < count; ++it) {
        string command = input::ReadLine();
        if (command[0] == 'B') {
            cout << command << ": " << transport.StatBus(command) << endl;
        } else if (command[0] == 'S') {
            cout << command << ": " << transport.StatStop(command) << endl;
        }
    }
}

} //end output
} //end transport_catalogue
