#include "transport_catalogue.h"
#include "geo.h"

#include <iterator>
#include <string>
#include <string_view>
#include <iostream>
#include <algorithm>

using namespace std;

namespace transport_catalogue {

void TransportCatalogue::AddDistance(Stop* stop_from, Stop* stop_to, int dist) {
    distance_[{stop_from, stop_to}] = dist;
}

void TransportCatalogue::AddStop(const Stop& stop_) {
    stops_.push_back(std::move(stop_));
    named_stops_[stops_.back().name] = &stops_.back();
    stops_buses_[&stops_.back()];
}

void TransportCatalogue::AddBus(const Bus& bus_) {
    buses_.push_back(bus_);
    named_buses_[buses_.back().name] = &buses_.back();
    for (auto& stop : buses_.back().route) {
        stops_buses_[stop].insert(buses_.back().name);
    }
}

Stop* TransportCatalogue::FindStop(string_view stop_name) const {
    return named_stops_.count(stop_name) ? named_stops_.at(stop_name) : nullptr;
}

Bus* TransportCatalogue::FindBus(string_view bus_name) const {
    return named_buses_.count(bus_name) ? named_buses_.at(bus_name) : nullptr;
}

int TransportCatalogue::FindDistance(Stop* stop_from, Stop* stop_to) const {
    if (distance_.count({stop_from, stop_to})) {
        return distance_.at({stop_from, stop_to});
    } else if (distance_.count({stop_to, stop_from})) {
        return distance_.at({stop_to, stop_from});
    }
    return 0;
}

StatBuses TransportCatalogue::ReturnStatBus(string_view bus_name) const {
    auto bus = FindBus(bus_name);
    if (bus) {
        int route = bus->route.size();
        vector<Stop*> unique;
        int length_distance = 0;
        double length_coordinate = 0.0;
        double curvature = 0.0;

        auto start = bus->route[0];
        for (auto& stop : bus->route) {
            if (find(unique.begin(), unique.end(), stop) == unique.end()) {
                unique.push_back(stop);
            }
            length_coordinate += ComputeDistance(start->coordinate, stop->coordinate);
            length_distance += FindDistance(start, stop);
            start = stop;
        }
        curvature = length_distance / length_coordinate;
        return {false, route, static_cast<int>(unique.size()), length_distance, curvature};
    }
    return {};
}

StatStops TransportCatalogue::ReturnStatStop(string_view stop_name) const {
    auto stop = FindStop(stop_name);
    if (stop) {
        return {false, stops_buses_.at(stop)};
    }
    return {};
}

const Stops& TransportCatalogue::GetStops() const {
    return stops_;
}

const Buses& TransportCatalogue::GetBuses() const {
    return buses_;
}

const NamedStops& TransportCatalogue::GetNamedStops() const {
    return named_stops_;
}

const NamedBuses& TransportCatalogue::GetNamedBuses() const {
    return named_buses_;
}

const StopsBuses& TransportCatalogue::GetStopsBuses() const {
    return stops_buses_;
}

const Distances& TransportCatalogue::GetDistanses() const {
    return distance_;
}

} //end transport_catalogue
