#pragma once

#include "graph.h"
#include "router.h"
#include "transport_catalogue.h"

#include <memory>
#include <unordered_map>
#include <optional>
#include <vector>

namespace transport_router {

struct RoutingSettings {
    int bus_wait_time = 0;
    double bus_velocity = 0.0;
};

struct RoutingStats {
    int count_stop = 0;
    int wait_time = 0;
    double travel_time = 0;

    bool operator<(const RoutingStats& other) const {
        return travel_time + wait_time < other.travel_time + other.wait_time;
    }

    bool operator>(const RoutingStats& other) const {
        return !(*this < other);
    }
};

RoutingStats operator+(const RoutingStats& lhs, const RoutingStats& rhs);

struct Routing {
    transport_catalogue::Stop* from = nullptr;
    transport_catalogue::Stop* to = nullptr;
    transport_catalogue::Bus* route = nullptr;

    RoutingStats stat = {};
};

using Route = std::vector<const Routing*>;

using RVertex = std::unordered_map<const transport_catalogue::Stop*, graph::VertexId>;
using REdge = std::vector<Routing>;
using U_Graph = std::unique_ptr<graph::DirectedWeightedGraph<RoutingStats>>;
using U_Route = std::unique_ptr<graph::Router<RoutingStats>>;

class TransportRouter {
private:
    const static int KiM = 1000;
    const static int HiM = 60;
public:
    
    TransportRouter(const transport_catalogue::TransportCatalogue& db) : db_(db) {}
    
    void SetSettings(RoutingSettings& settings);
    void SetGraph(graph::DirectedWeightedGraph<RoutingStats> graph);
    void SetRouter(graph::RoutesInternalData<RoutingStats> rid);
    void SetVexters(RVertex& vexters);

    void ConstructRouting();

    const transport_catalogue::TransportCatalogue& GetTransportCatalogue() const;
    RoutingSettings GetSettings() const;
    graph::DirectedWeightedGraph<RoutingStats> GetGraph() const;
    graph::Router<RoutingStats> GetRouter() const;
    RVertex GetVexters() const;
    REdge GetEdges() const;

    std::optional<Route> GetRoute(std::string_view from, std::string_view to) const;
private:
    const transport_catalogue::TransportCatalogue& db_;
    RoutingSettings settings_;

    U_Graph graph_ = nullptr;
    U_Route route_ = nullptr;

    RVertex vexters_;
    REdge edges_;

    void MarkStops(const transport_catalogue::NamedStops& stops);
    std::vector<int> MarkDistanse(const std::deque<transport_catalogue::Stop*>& route);
    void FillGraph(const transport_catalogue::NamedBuses& buses);

    void ConvertVelocity();
};

} // namespace transport_router
