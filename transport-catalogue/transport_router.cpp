#include "transport_router.h"
#include "graph.h"
#include "router.h"
#include "transport_catalogue.h"
#include <memory>
#include <optional>

namespace tc = transport_catalogue;

namespace transport_router {

RoutingStats operator+(const RoutingStats& lhs, const RoutingStats& rhs) {
    return {lhs.count_stop + rhs.count_stop,
            lhs.wait_time + rhs.wait_time,
            lhs.travel_time + rhs.travel_time
    };
}

void TransportRouter::ConvertVelocity() {
    settings_.bus_velocity = settings_.bus_velocity * KiM / HiM;
}

void TransportRouter::SetSettings(RoutingSettings& settings) {
    settings_ = settings;
    ConvertVelocity();
}

void TransportRouter::SetGraph(graph::DirectedWeightedGraph<RoutingStats> graph) {
    graph_ = std::make_unique<graph::DirectedWeightedGraph<RoutingStats>>(graph);
    FillGraph(db_.GetNamedBuses());
}

void TransportRouter::SetRouter(graph::RoutesInternalData<RoutingStats> rid) {
    route_ = std::make_unique<graph::Router<RoutingStats>>(*graph_, rid);
}

void TransportRouter::SetVexters(RVertex& vexters) {
    vexters_ = vexters;
}

const transport_catalogue::TransportCatalogue& TransportRouter::GetTransportCatalogue() const {
    return db_;
}

RoutingSettings TransportRouter::GetSettings() const {
    RoutingSettings settings = settings_;
    settings.bus_velocity = settings.bus_velocity * HiM / KiM; 
    return settings;
}

graph::DirectedWeightedGraph<RoutingStats> TransportRouter::GetGraph() const {
    return *graph_;
}

graph::Router<RoutingStats> TransportRouter::GetRouter() const {
    return *route_;
}

RVertex TransportRouter::GetVexters() const {
    return vexters_;
}

REdge TransportRouter::GetEdges() const {
    return edges_;
}

void TransportRouter::ConstructRouting() {
    const tc::NamedStops& stops = db_.GetNamedStops();
    MarkStops(stops);

    graph_ = std::make_unique<graph::DirectedWeightedGraph<RoutingStats>>(stops.size());

    const tc::NamedBuses& buses = db_.GetNamedBuses();
    FillGraph(buses);

    route_ = std::make_unique<graph::Router<RoutingStats>>(*graph_);
}

void TransportRouter::MarkStops(const tc::NamedStops& stops) {
    graph::VertexId count = 0;
    for (const auto& [name, stop] : stops) {
        vexters_.insert({stop, count++});
    }
}

void TransportRouter::FillGraph(const tc::NamedBuses& buses) {
    for (const auto& [name, bus] : buses) {
        const auto& route = bus->route;
        std::vector<int> distanses = MarkDistanse(route);
        for (int i = 0; i < route.size() - 1; ++i) {
            for (int j = i + 1; j < route.size(); ++j) {
                int distanse = 0;
                if (distanses[j] > distanses[i]) {
                    distanse = distanses[j] - distanses[i];
                } else {
                    distanse = distanses[i] - distanses[j];
                }
                RoutingStats stat{j - i, settings_.bus_wait_time, distanse / settings_.bus_velocity};
                Routing routing{route[i], route[j], bus, stat};

                graph_->AddEdge(graph::Edge<RoutingStats>{vexters_[routing.from], vexters_[routing.to], stat});
                edges_.emplace_back(std::move(routing));
            }
        }
    }
}

std::optional<Route> TransportRouter::GetRoute(std::string_view from, std::string_view to) const {
    tc::Stop* stop_from = db_.FindStop(from);
    tc::Stop* stop_to = db_.FindStop(to);

    if (stop_from == nullptr || stop_to == nullptr) {
        return std::nullopt;
    }

    graph::VertexId id_from = vexters_.at(stop_from);
    graph::VertexId id_to = vexters_.at(stop_to);

    Route result;

    if (stop_from == stop_to) {
        return result;
    }

    auto route = route_->BuildRoute(id_from, id_to);

    if (!route.has_value()) {
        return std::nullopt;
    }

    for (const auto& edge : route.value().edges) {
        result.emplace_back(&edges_.at(edge));
    }

    return result;
}

std::vector<int> TransportRouter::MarkDistanse(const std::deque<transport_catalogue::Stop*>& route) {
    int dist = 0;
    std::vector<int> result;
    result.push_back(0);
    for (int i = 0; i < route.size() - 1; ++i) {
        dist += db_.FindDistance(route[i], route[i + 1]);
        result.push_back(dist);
    }
    return result;
}

} // namespace transport_router
