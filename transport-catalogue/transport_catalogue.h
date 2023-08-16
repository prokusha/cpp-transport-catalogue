#pragma once

#include "geo.h"

#include <deque>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <string_view>
#include <string>
#include <set>
#include <iostream>

namespace transport_catalogue {

struct Stop {
    std::string name;
    detail::geo::Coordinates coordinate;
};

struct Bus {
    std::string name;
    std::deque<Stop*> route;
    std::unordered_set<Stop*> unique;
};

struct StatBuses {
    bool empty = true;
    int route;
    int unique;
    int length;
    double curvature;
};

struct StatStops {
    bool empty = true;
    bool empty_bus = true;
    std::set<std::string> name_bus;
};

struct Hasher {
public:
    std::size_t operator()(std::string_view name) const {
        return name_hash(name) * 75 + name_size_hash(name.size()) * 75 * 75;
    }
private:
    std::hash<std::string_view> name_hash;
    std::hash<int> name_size_hash;
};

struct HasherStop {
public:
    std::size_t operator()(Stop* stop) const {
        return name_hash(stop->name) * 75 * 75 * 75 +
            name_size_hash(stop->name.size()) * 75 * 75 * 75 * 75 +
            coord_hash(stop->coordinate.lat) * 75 * 75 * 75 * 75 * 75 +
            coord_hash(stop->coordinate.lng) * 75 * 75 * 75 * 75 * 75 * 75;
    }
private:
    std::hash<std::string_view> name_hash;
    std::hash<int> name_size_hash;
    std::hash<double> coord_hash;
};

struct HasherPair {
public:
    std::size_t operator()(std::pair<Stop*, Stop*> stop) const {
        return stop_hash(stop.first) * 75 * 75 * 75 * 75 * 75 * 75 * 75 +
        stop_hash(stop.second) * 75 * 75 * 75 * 75 * 75 * 75 * 75 * 75;
    }
private:
    std::hash<const void*> stop_hash;
};

class TransportCatalogue {
public:
    void MarkDistance();
    void AddStop(std::string_view);
    void AddBus(std::string_view);

    Stop* FindStop(std::string_view);
    Bus* FindBus(std::string_view);
    int FindDistance(Stop*, Stop*);

    StatBuses StatBus(std::string_view);
    StatStops StatStop(std::string_view);
private:
    std::deque<Stop> stops_;
    std::deque<Bus> buses_;
    std::vector<std::pair<Stop*, std::string_view>> distance_temp_;

    std::unordered_map<std::string_view, Stop*, Hasher> named_stops_;
    std::unordered_map<std::string_view, Bus*, Hasher> named_buses_;
    std::unordered_map<Stop*, std::set<std::string>, HasherStop> stops_buses_;
    std::unordered_map<std::pair<Stop*, Stop*>, int, HasherPair> distance_;
};

} //end transport_catalogue
