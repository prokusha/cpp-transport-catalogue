#include "serialization.h"
#include "graph.h"
#include "router.h"
#include "transport_catalogue.h"
#include "transport_router.h"
#include "transport_router.pb.h"
#include <optional>

namespace serialization {

transport_catalogue_protobuf::TransportCatalogue TransportCatalogueSerialization(const transport_catalogue::TransportCatalogue& TransportCatalogue) {
    transport_catalogue_protobuf::TransportCatalogue TransportCatalogue_proto;

    const transport_catalogue::Stops& stops = TransportCatalogue.GetStops();
    const transport_catalogue::Buses& buses = TransportCatalogue.GetBuses();
    const transport_catalogue::Distances& distances = TransportCatalogue.GetDistanses();

    for (int i = 0; i < stops.size(); ++i) {
        transport_catalogue_protobuf::Stop stop;
        
        stop.set_id(i);
        stop.set_name(stops[i].name);
        stop.set_lat(stops[i].coordinate.lat);
        stop.set_lng(stops[i].coordinate.lng);

        *TransportCatalogue_proto.add_stops() = std::move(stop);
    }

    for (int i = 0; i < buses.size(); ++i) {
        transport_catalogue_protobuf::Bus bus;

        bus.set_name(buses[i].name);
        bus.set_is_roundtrip(buses[i].is_roundtrip);

        for (int j = 0; j < buses[i].route.size(); ++j) {
            bus.add_stops(CalculateIdStop(stops.begin(), stops.end(), buses[i].route[j]->name));
        }
        *TransportCatalogue_proto.add_buses() = std::move(bus);
    }

    for (const auto& [from_to, dist] : distances) {
        transport_catalogue_protobuf::Distance distance;
        
        distance.set_from(CalculateIdStop(stops.begin(), stops.end(), from_to.first->name));
        distance.set_to(CalculateIdStop(stops.begin(), stops.end(), from_to.second->name));
        distance.set_distance(dist);

        *TransportCatalogue_proto.add_distances() = std::move(distance);
    }

    return TransportCatalogue_proto;
}

transport_catalogue::TransportCatalogue TransportCatalogueDeserialization(const transport_catalogue_protobuf::TransportCatalogue& TransportCatalogue_proto) {
    transport_catalogue::TransportCatalogue TransportCatalogue;

    const auto& stops_proto = TransportCatalogue_proto.stops();
    const auto& buses_proto = TransportCatalogue_proto.buses();
    const auto& distances_proto = TransportCatalogue_proto.distances();

    for (const auto& stop_proto : stops_proto) {
        transport_catalogue::Stop stop;

        stop.name = stop_proto.name();
        stop.coordinate.lat = stop_proto.lat();
        stop.coordinate.lng = stop_proto.lng();

        TransportCatalogue.AddStop(std::move(stop));
    }

    const transport_catalogue::Stops& stops = TransportCatalogue.GetStops(); 
    const transport_catalogue::NamedStops& named_stops = TransportCatalogue.GetNamedStops();
    
    for (const auto& bus_proto : buses_proto) {
        transport_catalogue::Bus bus;

        bus.name = bus_proto.name();
        bus.is_roundtrip = bus_proto.is_roundtrip();
        
        for (auto& stop : bus_proto.stops()) {
            bus.route.push_back(named_stops.at(stops[stop].name));
        }

        TransportCatalogue.AddBus(std::move(bus));
    }
    
    for (const auto& distance_proto : distances_proto) {
        transport_catalogue::Stop* from = named_stops.at(stops[distance_proto.from()].name);
        transport_catalogue::Stop* to = named_stops.at(stops[distance_proto.to()].name);
        TransportCatalogue.AddDistance(from, to, distance_proto.distance());
    }
    
    return TransportCatalogue;
}

transport_catalogue_protobuf::TransportRouter TransportRouterSerialization(const transport_router::TransportRouter& TransportRouter) {
    transport_catalogue_protobuf::TransportRouter TransportRouter_proto;

    transport_catalogue_protobuf::RoutingSettings settings_proto;
    auto settings = TransportRouter.GetSettings();

    settings_proto.set_bus_wait_time(settings.bus_wait_time);
    settings_proto.set_bus_velocity(settings.bus_velocity);

    *TransportRouter_proto.mutable_settings() = std::move(settings_proto);

    const transport_catalogue::Stops& stops = TransportRouter.GetTransportCatalogue().GetStops();

    const transport_catalogue::Buses& buses = TransportRouter.GetTransportCatalogue().GetBuses();

    transport_catalogue_protobuf::Router Router_proto;
    for (auto& routes_internal_data : TransportRouter.GetRouter().GetRoutes()) {
		transport_catalogue_protobuf::RouteInternalDataArr new_array;
		for (auto& route_internal_data : routes_internal_data){
			transport_catalogue_protobuf::RouteInternalData new_struct;

			if (route_internal_data.has_value()){
				new_struct.set_has_value(true);
				*new_struct.mutable_weight() = RoutingStatsSerialization(route_internal_data->weight);
				if (route_internal_data->prev_edge.has_value()){
					new_struct.mutable_edge_id()->set_has_value(true);
					new_struct.mutable_edge_id()->set_edge_id(route_internal_data.value().prev_edge.value());
				} else {
					new_struct.mutable_edge_id()->set_has_value(false);
				}
			} else {
				new_struct.set_has_value(false);
			}
            *new_array.add_data() = std::move(new_struct);
		}
        *Router_proto.add_data() = std::move(new_array);
	}
    *TransportRouter_proto.mutable_router_data() = Router_proto;

    return TransportRouter_proto;
}

TransportRouter TransportRouterDeserialization(const transport_catalogue::TransportCatalogue& TransportCatalogue, const transport_catalogue_protobuf::TransportRouter& TransportRouter_proto) {
    TransportRouter TransportRouter;
    
    transport_catalogue_protobuf::RoutingSettings settings_proto = TransportRouter_proto.settings();
    transport_router::RoutingSettings settings;

    settings.bus_velocity = settings_proto.bus_velocity();
    settings.bus_wait_time = settings_proto.bus_wait_time();

    TransportRouter.settings = std::move(settings);

    const transport_catalogue::Stops& stops = TransportCatalogue.GetStops();
    transport_router::RVertex vexters;

    const auto& named_stops = TransportCatalogue.GetNamedStops();
    graph::VertexId id = 0;
    for (const auto& [name, stop] : named_stops) {
        vexters.insert({stop, id++});
    }

    TransportRouter.vexters = std::move(vexters);

    const auto& named_buses = TransportCatalogue.GetNamedBuses();
    const transport_catalogue::Buses& buses = TransportCatalogue.GetBuses();

    graph::RoutesInternalData<transport_router::RoutingStats> routes_internal_data(TransportRouter.vexters.size());

	for (int i = 0; i < TransportRouter_proto.router_data().data_size(); ++i){
		std::vector<std::optional<graph::RouteInternalData<transport_router::RoutingStats>>> new_arr;

		for (int j = 0; j < TransportRouter_proto.router_data().data(i).data_size(); ++j){
			
			if (TransportRouter_proto.router_data().data(i).data(j).has_value()){
				graph::RouteInternalData<transport_router::RoutingStats> new_struct;
				if (TransportRouter_proto.router_data().data(i).data(j).edge_id().has_value()){
                    new_struct.weight = RoutingStatsDeserialization(TransportRouter_proto.router_data().data(i).data(j).weight());
					new_struct.prev_edge = TransportRouter_proto.router_data().data(i).data(j).edge_id().edge_id();
				} else {
					new_struct.weight = RoutingStatsDeserialization(TransportRouter_proto.router_data().data(i).data(j).weight());
					new_struct.prev_edge = std::nullopt;
				}
				new_arr.push_back(new_struct);
			} else {
				new_arr.push_back(std::nullopt);
			}

		}
        routes_internal_data[i] = new_arr;
    }

    TransportRouter.routes_internal_data = std::move(routes_internal_data);

    return TransportRouter;
}

transport_catalogue_protobuf::Color ColorSerialization(const svg::Color& Color) {
    transport_catalogue_protobuf::Color Color_proto;
    
    if (std::holds_alternative<std::monostate>(Color)) {
        Color_proto.set_none(true);
    } else if (std::holds_alternative<svg::Rgb>(Color)) {
        svg::Rgb rgb = std::get<svg::Rgb>(Color);

        Color_proto.mutable_rgb()->set_red(rgb.red);
        Color_proto.mutable_rgb()->set_green(rgb.green);
        Color_proto.mutable_rgb()->set_blue(rgb.blue);
    } else if (std::holds_alternative<svg::Rgba>(Color)) {
        svg::Rgba rgba = std::get<svg::Rgba>(Color);

        Color_proto.mutable_rgba()->set_red(rgba.red);
        Color_proto.mutable_rgba()->set_green(rgba.green);
        Color_proto.mutable_rgba()->set_blue(rgba.blue);
        Color_proto.mutable_rgba()->set_opacity(rgba.opacity);
    } else if (std::holds_alternative<std::string>(Color)) {
        Color_proto.set_string_color(std::get<std::string>(Color));
    }

    return Color_proto;
}

svg::Color ColorDeserialization(const transport_catalogue_protobuf::Color& Color_proto) {
    svg::Color Color;

    if (Color_proto.has_rgb()) {
        svg::Rgb rgb;

        rgb.red = Color_proto.rgb().red();
        rgb.green = Color_proto.rgb().green();
        rgb.blue = Color_proto.rgb().blue();

        Color = rgb;
    } else if (Color_proto.has_rgba()) {
        svg::Rgba rgba;

        rgba.red = Color_proto.rgba().red();
        rgba.green = Color_proto.rgba().green();
        rgba.blue = Color_proto.rgba().blue();
        rgba.opacity = Color_proto.rgba().opacity();

        Color = rgba;
    } else {
        Color = Color_proto.string_color();
    }

    return Color;
}

transport_catalogue_protobuf::Point PointSerialization(const svg::Point& Point) {
    transport_catalogue_protobuf::Point Point_proto;
    
    Point_proto.set_x(Point.x);
    Point_proto.set_y(Point.y);

    return Point_proto;
}

svg::Point PointDeserialization(const transport_catalogue_protobuf::Point& Point_proto) {
    svg::Point Point;

    Point.x = Point_proto.x();
    Point.y = Point_proto.y();

    return Point;
}

transport_catalogue_protobuf::MapSettings MapSettingsSerialization(const renderer::MapSettings& MapSettings) {
    transport_catalogue_protobuf::MapSettings MapSettings_proto;

    MapSettings_proto.set_width(MapSettings.width);
    MapSettings_proto.set_height(MapSettings.height);
    MapSettings_proto.set_padding(MapSettings.padding);
    MapSettings_proto.set_line_width(MapSettings.line_width);
    MapSettings_proto.set_stop_radius(MapSettings.stop_radius);

    MapSettings_proto.set_bus_label_font_size(MapSettings.bus_label_font_size);
    *MapSettings_proto.mutable_bus_label_offset() = std::move(PointSerialization(MapSettings.bus_label_offset));

    MapSettings_proto.set_stop_label_font_size(MapSettings.stop_label_font_size);
    *MapSettings_proto.mutable_stop_label_offset() = std::move(PointSerialization(MapSettings.stop_label_offset));

    *MapSettings_proto.mutable_underlayer_color() = std::move(ColorSerialization(MapSettings.underlayer_color));
    MapSettings_proto.set_underlayer_width(MapSettings.underlayer_width);

    const auto& colors = MapSettings.color_palette;

    for (const svg::Color& color : colors) {
        *MapSettings_proto.add_color_palette() = std::move(ColorSerialization(color));
    }

    return MapSettings_proto;
}

renderer::MapSettings MapSettingsDeserialization(const transport_catalogue_protobuf::MapSettings& MapSettings_proto) {
    renderer::MapSettings MapSettings;

    MapSettings.width = MapSettings_proto.width();
    MapSettings.height = MapSettings_proto.height();
    MapSettings.padding = MapSettings_proto.padding();
    MapSettings.line_width = MapSettings_proto.line_width();
    MapSettings.stop_radius = MapSettings_proto.stop_radius();
    
    MapSettings.bus_label_font_size = MapSettings_proto.bus_label_font_size();
    MapSettings.bus_label_offset = std::move(PointDeserialization(MapSettings_proto.bus_label_offset()));
    
    MapSettings.stop_label_font_size = MapSettings_proto.stop_label_font_size();
    MapSettings.stop_label_offset = std::move(PointDeserialization(MapSettings_proto.stop_label_offset()));
    
    MapSettings.underlayer_color = std::move(ColorDeserialization(MapSettings_proto.underlayer_color()));
    MapSettings.underlayer_width = MapSettings_proto.underlayer_width();

    for (const transport_catalogue_protobuf::Color& color_proto : MapSettings_proto.color_palette()) {
        MapSettings.color_palette.push_back(std::move(ColorDeserialization(color_proto)));
    }

    MapSettings.color = MapSettings.color_palette[0];

    return MapSettings;
}

transport_catalogue_protobuf::RoutingStats RoutingStatsSerialization(const transport_router::RoutingStats& RoutingStats) {
    transport_catalogue_protobuf::RoutingStats RoutingStats_proto;
    
    RoutingStats_proto.set_count_stop(RoutingStats.count_stop);
    RoutingStats_proto.set_wait_time(RoutingStats.wait_time);
    RoutingStats_proto.set_travel_time(RoutingStats.travel_time);

    return RoutingStats_proto;
}

transport_router::RoutingStats RoutingStatsDeserialization(const transport_catalogue_protobuf::RoutingStats& RoutingStats_proto) {
    transport_router::RoutingStats RoutingStats;
    
    RoutingStats.count_stop = RoutingStats_proto.count_stop();
    RoutingStats.travel_time = RoutingStats_proto.travel_time();
    RoutingStats.wait_time = RoutingStats_proto.wait_time();

    return RoutingStats;
}

transport_catalogue_protobuf::DirectedWeightedGraph GraphSerialization(const graph::DirectedWeightedGraph<transport_router::RoutingStats>& Graph) {
    transport_catalogue_protobuf::DirectedWeightedGraph Graph_proto;

    for (const graph::Edge<transport_router::RoutingStats> edge : Graph.GetEdges()) {
        transport_catalogue_protobuf::Edge Edge_proto;

        Edge_proto.set_from(edge.from);
        Edge_proto.set_to(edge.to);
        *Edge_proto.mutable_weigth() = RoutingStatsSerialization(edge.weight);

        *Graph_proto.add_edges() = std::move(Edge_proto);
    }

    for (const std::vector<graph::EdgeId> incidence : Graph.GetIncidenceList()) {
        transport_catalogue_protobuf::incidence incidence_proto;
        for (const graph::EdgeId id : incidence) {
            incidence_proto.add_edgeid(id);
        }
        *Graph_proto.add_incidence_lists() = incidence_proto;
    }

    return Graph_proto;
}

graph::DirectedWeightedGraph<transport_router::RoutingStats> GraphDeserialization(const transport_catalogue_protobuf::DirectedWeightedGraph& Graph_proto) {
    graph::DirectedWeightedGraph<transport_router::RoutingStats> Graph(Graph_proto.edges_size());

    std::vector<graph::Edge<transport_router::RoutingStats>> Edges;
    for (const transport_catalogue_protobuf::Edge& edge_proto : Graph_proto.edges()) {
        graph::Edge<transport_router::RoutingStats> Edge;

        Edge.from = edge_proto.from();
        Edge.to = edge_proto.to();
        Edge.weight = RoutingStatsDeserialization(edge_proto.weigth());

        Edges.push_back(Edge);
    }
    Graph.SetEdges(std::move(Edges));

    std::vector<graph::IncidenceList> incidence_list;
    for (const transport_catalogue_protobuf::incidence& incidence_proto : Graph_proto.incidence_lists()) {
        std::vector<graph::EdgeId> incidence;
        for (const int id : incidence_proto.edgeid()) {
            incidence.push_back(id);
        }
        incidence_list.push_back(incidence);
    }
    Graph.SetIncidence(std::move(incidence_list));

    return Graph;
}

void CatalogueSerialization(transport_catalogue::TransportCatalogue& TransportCatalogue, renderer::MapSettings& MapSettings, transport_router::TransportRouter& TransportRouter, std::ostream& out) {
    transport_catalogue_protobuf::Catalogue Catalogue_proto;

    transport_catalogue_protobuf::TransportCatalogue TransportCatalogue_proto = TransportCatalogueSerialization(TransportCatalogue);
    
    transport_catalogue_protobuf::MapSettings MapSettings_proto = MapSettingsSerialization(MapSettings);

    transport_catalogue_protobuf::DirectedWeightedGraph Graph_proto = GraphSerialization(TransportRouter.GetGraph());

    transport_catalogue_protobuf::TransportRouter TransportRouter_proto = TransportRouterSerialization(TransportRouter);

    *Catalogue_proto.mutable_transport_catalogue() = TransportCatalogue_proto;
    *Catalogue_proto.mutable_map_settings() = MapSettings_proto;
    *Catalogue_proto.mutable_graph() = Graph_proto;
    *Catalogue_proto.mutable_transport_router() = TransportRouter_proto;

    Catalogue_proto.SerializePartialToOstream(&out);
}

Catalogue CatalogueDeserialization(std::istream& in) {
    transport_catalogue_protobuf::Catalogue Catalogue_proto;
    const auto& check = Catalogue_proto.ParseFromIstream(&in);
    if (!check) {
        throw std::runtime_error("parse istream error");
    }

    transport_catalogue::TransportCatalogue transport_catalogue = TransportCatalogueDeserialization(Catalogue_proto.transport_catalogue());
    renderer::MapSettings map_settings = MapSettingsDeserialization(Catalogue_proto.map_settings());
    
    graph::DirectedWeightedGraph<transport_router::RoutingStats> graph = GraphDeserialization(Catalogue_proto.graph());
    
    TransportRouter transport_router = TransportRouterDeserialization(transport_catalogue, Catalogue_proto.transport_router());
    TransportRouterData transport_router_data;
    
    transport_router_data.transport_router = std::move(transport_router);
    transport_router_data.graph = std::move(graph);

    return {std::move(transport_catalogue), std::move(map_settings), std::move(transport_router_data)};
}

} // end serialization
