#include "input_reader.h"
#include "transport_catalogue.h"

#include <iostream>
#include <algorithm>
#include <string_view>

using namespace std;

vector<string> bus_temp;
vector<pair<string, string>> distance_temp;

namespace transport_catalogue {
namespace detail {
namespace string_refactoring {

string MakeName(string_view& str) {
    string name = {str.begin() + str.find(' ') + 1, str.begin() + str.find(':')};
    str.remove_prefix(str.find(':') + 2);
    return name;
}

string_view::iterator FindSimbol(string_view str, char simbol) {
    return find(str.begin(), str.end(), simbol);
}

Stop MakeStop(string_view command) {
    Stop stop_;

    stop_.name = detail::string_refactoring::MakeName(command);

    stop_.coordinate.lat = stod(string{command.begin(), detail::string_refactoring::FindSimbol(command, ',')});
    command.remove_prefix(command.find(',') + 2);
    stop_.coordinate.lng = stod(string{command.begin(), detail::string_refactoring::FindSimbol(command, ',')});

    if (detail::string_refactoring::FindSimbol(command, ',') != command.end()) {
        command.remove_prefix(command.find(',') + 2);
        distance_temp.push_back({stop_.name, string(command)});
    }

    return stop_;
}

Bus MakeBus(string_view command, TransportCatalogue& transport) {
    Bus bus_;
    bus_.name = detail::string_refactoring::MakeName(command);

    char find_;
    bool reverse_ = false;
    Stop* stop = nullptr;

    if (detail::string_refactoring::FindSimbol(command, '>') != command.end()) {
        find_ = '>';
    } else {
        reverse_ = true;
        find_ = '-';
    }

    while(detail::string_refactoring::FindSimbol(command, find_) != command.end()) {
        stop = transport.FindStop(string{command.begin(), detail::string_refactoring::FindSimbol(command, find_) - 1});
        bus_.unique += count(bus_.route.begin(), bus_.route.end(), stop) >= 1 ? 0 : 1;
        bus_.route.push_back(stop);
        command.remove_prefix(command.find(find_) + 2);
    }
    stop = transport.FindStop(command);
    bus_.unique += count(bus_.route.begin(), bus_.route.end(), stop) < 1 ? 1 : 0;
    bus_.route.push_back(stop);

    if (reverse_) {
        auto temp = bus_.route;
        reverse(temp.begin(), temp.end());
        bus_.route.insert(bus_.route.end(), next(temp.begin()), temp.end());
    }
    return bus_;
}

void MarkDistance(TransportCatalogue& transport) {
    int dist = 0;
    Stop* x = nullptr;
    Stop* y = nullptr;
    for (auto& [stop_from, command] : distance_temp) {
        string_view sv = command;
        x = transport.FindStop(stop_from);
        while (FindSimbol(sv, ',') != sv.end()) {
            dist = stoi(string{sv.begin(), detail::string_refactoring::FindSimbol(sv, 'm')});
            sv.remove_prefix(sv.find('m') + 5);
            y = transport.FindStop(string{sv.begin(), detail::string_refactoring::FindSimbol(sv, ',')});
            transport.AddDistance({x, y}, dist);
            sv.remove_prefix(sv.find(',') + 1);
        }
        dist = stoi(string{sv.begin(), detail::string_refactoring::FindSimbol(sv, 'm')});
        sv.remove_prefix(sv.find('m') + 5);
        y = transport.FindStop(string{sv});
        transport.AddDistance({x, y}, dist);
    }
}

} //end string_refactoring
} //end detail

namespace input {

string ReadLine() {
    string s;
    getline(cin, s);
    return s;
}

int ReadLineWithNumber() {
    int result;
    cin >> result;
    ReadLine();
    return result;
}

void Add(TransportCatalogue& transport) {
    int count = ReadLineWithNumber();
    for (int it = 0; it < count; ++it) {
        string command = ReadLine();
        if (command[0] == 'S') {
            transport.AddStop(std::move(detail::string_refactoring::MakeStop(command)));
        } else {
            bus_temp.push_back(std::move(command));
        }
    }

    for (auto& command : bus_temp) {
        transport.AddBus(std::move(detail::string_refactoring::MakeBus(command, transport)));
    }

    detail::string_refactoring::MarkDistance(transport);
}

} //end input
} //end transport_catalogue
