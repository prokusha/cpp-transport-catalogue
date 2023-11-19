#pragma once

#include "transport_catalogue.h"
#include "transport_catalogue.pb.h"

#include "map_renderer.h"
#include "map_renderer.pb.h"

#include "svg.pb.h"

#include "router.h"

#include "graph.h"
#include "graph.pb.h"

#include "transport_router.h"
#include "transport_router.pb.h"

#include <iostream>

namespace serialization {

struct serialization_settings {
    std::string name_file;
};

struct TransportRouter {
    transport_router::RoutingSettings settings;
    transport_router::RVertex vexters;
    graph::RoutesInternalData<transport_router::RoutingStats> routes_internal_data;
};

struct TransportRouterData {
    TransportRouter transport_router;
    graph::DirectedWeightedGraph<transport_router::RoutingStats> graph;
};

struct Catalogue {
    transport_catalogue::TransportCatalogue transport_catalogue;
    renderer::MapSettings map_settings;
    TransportRouterData transport_router_data;
};

template <typename It>
uint32_t CalculateIdStop(It start, It end, std::string_view name) {
 
    auto stop_it = std::find_if(start, end, [&name](const transport_catalogue::Stop stop) {return stop.name == name;});
    return std::distance(start, stop_it);
}

template <typename It>
uint32_t CalculateIdBus(It start, It end, std::string_view name) {
 
    auto bus_it = std::find_if(start, end, [&name](const transport_catalogue::Bus bus) {return bus.name == name;});
    return std::distance(start, bus_it);
}

transport_catalogue_protobuf::TransportCatalogue TransportCatalogueSerialization(const transport_catalogue::TransportCatalogue& TransportCatalogue);
transport_catalogue::TransportCatalogue TransportCatalogueDeserialization(const transport_catalogue_protobuf::TransportCatalogue& TransportCatalogue_proto);

transport_catalogue_protobuf::TransportRouter TransportRouterSerialization(const transport_router::TransportRouter& TransportRouter);
TransportRouter TransportRouterDeserialization(const transport_catalogue::TransportCatalogue& TransportCatalogue, const transport_catalogue_protobuf::TransportRouter& TransportRouter_proto);

transport_catalogue_protobuf::Color ColorSerialization(const svg::Color& Color);
svg::Color ColorDeserialization(const transport_catalogue_protobuf::Color& Color_proto);

transport_catalogue_protobuf::Point PointSerialization(const svg::Point& Point);
svg::Point PointDeserialization(const transport_catalogue_protobuf::Point& Point_proto);

transport_catalogue_protobuf::MapSettings MapSettingsSerialization(const renderer::MapSettings& MapSettings);
renderer::MapSettings MapSettingsDeserialization(const transport_catalogue_protobuf::MapSettings& MapSettings_proto);

transport_catalogue_protobuf::RoutingStats RoutingStatsSerialization(const transport_router::RoutingStats& RoutingStats);
transport_router::RoutingStats RoutingStatsDeserialization(const transport_catalogue_protobuf::RoutingStats& RoutingStats_proto);

transport_catalogue_protobuf::DirectedWeightedGraph GraphSerialization(const graph::DirectedWeightedGraph<transport_router::RoutingStats>& Graph);
graph::DirectedWeightedGraph<transport_router::RoutingStats> GraphDeserialization(const transport_catalogue_protobuf::DirectedWeightedGraph& Graph_proto);

void CatalogueSerialization(transport_catalogue::TransportCatalogue& TransportCatalogue, renderer::MapSettings& MapSettings, transport_router::TransportRouter& TransportRouter, std::ostream& out);
Catalogue CatalogueDeserialization(std::istream& in); 

} // end serialization