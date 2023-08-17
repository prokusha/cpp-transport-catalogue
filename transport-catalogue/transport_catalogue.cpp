#include "transport_catalogue.h"
#include "geo.h"

#include <iterator>
#include <string>
#include <string_view>
#include <iostream>
#include <algorithm>

using namespace std;

namespace transport_catalogue {

void TransportCatalogue::AddDistance(pair<Stop*, Stop*> stops_from_to, int dist) {
    distance_[stops_from_to] = dist;
}

void TransportCatalogue::AddStop(Stop stop_) {
    stops_.push_back(std::move(stop_));
    named_stops_[stops_.back().name] = &stops_.back();
    stops_buses_[&stops_.back()];
}

void TransportCatalogue::AddBus(Bus bus_) {
    buses_.push_back(bus_);
    named_buses_[buses_.back().name] = &buses_.back();
    for (auto& stop : buses_.back().route) {
        stops_buses_[stop].insert(buses_.back().name);
    }
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

StatBuses TransportCatalogue::ReturnStatBus(string_view bus_name) {
    bus_name.remove_prefix(bus_name.find(' ') + 1);
    auto bus = FindBus(bus_name);
    if (bus) {
        int route = bus->route.size();
        int unique = bus->unique;
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

StatStops TransportCatalogue::ReturnStatStop(string_view stop_name) {
    stop_name.remove_prefix(stop_name.find(' ') + 1);
    auto stop = FindStop(stop_name);
    if (stop) {
        return {false, stops_buses_.at(stop)};
    }
    return {};
}

} //end transport_catalogue
