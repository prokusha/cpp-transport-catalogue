#pragma once

#include "geo.h"

#include <deque>
#include <vector>
#include <unordered_map>
#include <string_view>
#include <string>
#include <set>

namespace transport_catalogue {

struct Stop {
    std::string name;
    detail::geo::Coordinates coordinate;
};

struct Bus {
    std::string name;
    std::deque<Stop*> route;
    bool is_roundtrip;
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
        return name_hash(stop->name) +
            name_size_hash(stop->name.size()) +
            coord_hash(stop->coordinate.lat) * 75 +
            coord_hash(stop->coordinate.lng) * 75 * 75;
    }
private:
    std::hash<std::string_view> name_hash;
    std::hash<int> name_size_hash;
    std::hash<double> coord_hash;
};

struct HasherPair {
public:
    std::size_t operator()(std::pair<Stop*, Stop*> stop) const {
        return stop_hash(stop.first) * 75 +
        stop_hash(stop.second) * 75 * 75;
    }
private:
    std::hash<const void*> stop_hash;
};

using Stops = std::deque<Stop>;
using Buses = std::deque<Bus>;

using NamedStops = std::unordered_map<std::string_view, Stop*, Hasher>;
using NamedBuses = std::unordered_map<std::string_view, Bus*, Hasher>;
using StopsBuses = std::unordered_map<Stop*, std::set<std::string>, HasherStop>;
using Distances = std::unordered_map<std::pair<Stop*, Stop*>, int, HasherPair>;

class TransportCatalogue {
public:

    void AddDistance(Stop* stop_from, Stop* stop_to, int dist);
    void AddStop(const Stop& stop_);
    void AddBus(const Bus& bus_);

    Stop* FindStop(std::string_view stop_name) const;
    Bus* FindBus(std::string_view bus_name) const;
    int FindDistance(Stop* stop_from, Stop* stop_to) const;

    StatBuses ReturnStatBus(std::string_view bus_name) const;
    StatStops ReturnStatStop(std::string_view stop_name) const;

    const Stops& GetStops() const;
    const Buses& GetBuses() const;

    const NamedStops& GetNamedStops() const;
    const NamedBuses& GetNamedBuses() const;

    const StopsBuses& GetStopsBuses() const;
    const Distances& GetDistanses() const;

private:
    Stops stops_;
    Buses buses_;
    std::vector<std::pair<Stop*, std::string_view>> distance_temp_;

    NamedStops named_stops_;
    NamedBuses named_buses_;
    StopsBuses stops_buses_;
    Distances distance_;
};

} //end transport_catalogue
