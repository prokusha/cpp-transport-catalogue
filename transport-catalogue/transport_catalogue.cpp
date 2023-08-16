#include "transport_catalogue.h"
#include "geo.h"

#include <iterator>
#include <string>
#include <string_view>
#include <iostream>
#include <algorithm>

using namespace std;

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

} // end string_refactoring
} //end detail

void TransportCatalogue::MarkDistance() {
    int dist;
    Stop* stop_ = nullptr;
    for (auto& [stop, distance] : distance_temp_) {
        while (detail::string_refactoring::FindSimbol(distance, ',') != distance.end()) {
            dist = stoi(string{distance.begin(), detail::string_refactoring::FindSimbol(distance, 'm')});
            distance.remove_prefix(distance.find('m') + 5);
            stop_ = FindStop(string{distance.begin(), detail::string_refactoring::FindSimbol(distance, ',')});
            distance_[{stop, stop_}] = dist;
            distance.remove_prefix(distance.find(',') + 1);
        }
        dist = stoi(string{distance.begin(), detail::string_refactoring::FindSimbol(distance, 'm')});
        distance.remove_prefix(distance.find('m') + 5);
        stop_ = FindStop(string(distance));
        distance_[{stop, stop_}] = dist;
    }
}

void TransportCatalogue::AddStop(string_view stop) {
    Stop stop_;
    stop_.name = detail::string_refactoring::MakeName(stop);

    stop_.coordinate.lat = stod(string{stop.begin(), detail::string_refactoring::FindSimbol(stop, ',')});
    stop.remove_prefix(stop.find(',') + 2);
    stop_.coordinate.lng = stod(string{stop.begin(), detail::string_refactoring::FindSimbol(stop, ',')});

    stops_.push_back(std::move(stop_));
    named_stops_[stops_.back().name] = &stops_.back();
    stops_buses_[&stops_.back()];

    if (detail::string_refactoring::FindSimbol(stop, ',') != stop.end()) {
        stop.remove_prefix(stop.find(',') + 2);
        distance_temp_.push_back({&stops_.back(), stop});
    }
}

void TransportCatalogue::AddBus(string_view bus) {
    Bus bus_;
    bus_.name = detail::string_refactoring::MakeName(bus);

    char find_;
    bool reverse_ = false;

    if (detail::string_refactoring::FindSimbol(bus, '>') != bus.end()) {
        find_ = '>';
    } else {
        reverse_ = true;
        find_ = '-';
    }

    while(detail::string_refactoring::FindSimbol(bus, find_) != bus.end()) {
        bus_.route.push_back(FindStop(string{bus.begin(), detail::string_refactoring::FindSimbol(bus, find_) - 1}));
        bus_.unique.insert(bus_.route.back());
        stops_buses_[bus_.route.back()].insert(bus_.name);
        bus.remove_prefix(bus.find(find_) + 2);
    }

    bus_.route.push_back(FindStop(bus));
    bus_.unique.insert(bus_.route.back());
    stops_buses_[bus_.route.back()].insert(bus_.name);

    if (reverse_) {
        auto temp = bus_.route;
        reverse(temp.begin(), temp.end());
        bus_.route.insert(bus_.route.end(), next(temp.begin()), temp.end());
    }

    buses_.push_back(bus_);
    named_buses_[buses_.back().name] = &buses_.back();
}

Stop* TransportCatalogue::FindStop(string_view stop_name) {
    return named_stops_.count(stop_name) ? named_stops_.at(stop_name) : nullptr;
}

Bus* TransportCatalogue::FindBus(string_view bus_name) {
    return named_buses_.count(bus_name) ? named_buses_.at(bus_name) : nullptr;
}

int TransportCatalogue::FindDistance(Stop* x, Stop* y) {
    if (distance_.count({x, y})) {
        return distance_.at({x, y});
    } else if (distance_.count({y, x})) {
        return distance_.at({y, x});
    }
    return 0;
}

StatBuses TransportCatalogue::StatBus(string_view bus_name) {
    bus_name.remove_prefix(bus_name.find(' ') + 1);
    auto bus = FindBus(bus_name);
    if (bus) {
        int route = bus->route.size();
        int unique = bus->unique.size();
        int length_distance = 0;
        double length_coordinate = 0.0;
        double curvature = 0.0;

        auto start = bus->route[0];
        for (auto& stop : bus->route) {
            length_coordinate += ComputeDistance(start->coordinate, stop->coordinate);
            length_distance += FindDistance(start, stop);
            start = stop;
        }
        curvature = length_distance / length_coordinate;
        return {false, route, unique, length_distance, curvature};
    }
    return {};
}

StatStops TransportCatalogue::StatStop(string_view stop_name) {
    stop_name.remove_prefix(stop_name.find(' ') + 1);
    auto stop = FindStop(stop_name);
    if (stop) {
        return {false, stops_buses_[stop].empty(), stops_buses_.at(stop)};
    }
    return {};
}

} //end transport_catalogue
