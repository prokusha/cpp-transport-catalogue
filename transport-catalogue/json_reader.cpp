#include "json_reader.h"
#include "geo.h"
#include "json.h"
#include "map_renderer.h"
#include "serialization.h"
#include "svg.h"
#include "transport_catalogue.h"
#include "transport_router.h"

#include <algorithm>
#include <iostream>
#include <sstream>

using namespace transport_catalogue;
using namespace transport_router;

namespace json_reader {

namespace detail {

Stop Maker::MakeStop(const json::Dict& jstop) {
    Stop stop;

    stop.name = jstop.at("name").AsString();
    stop.coordinate.lat = jstop.at("latitude").AsDouble();
    stop.coordinate.lng = jstop.at("longitude").AsDouble();
    distance_.push_back({stop.name, jstop.at("road_distances")});

    return stop;
}

Bus Maker::MakeBus(transport_catalogue::TransportCatalogue& db, const json::Dict& jbus) {
    Bus bus;

    bus.name = jbus.at("name").AsString();

    for (const auto& stop : jbus.at("stops").AsArray()) {
        bus.route.push_back(db.FindStop(stop.AsString()));
    }

    bus.is_roundtrip = jbus.at("is_roundtrip").AsBool();

    if (!bus.is_roundtrip) {
        auto temp = bus.route;
        std::reverse(temp.begin(), temp.end());
        bus.route.insert(bus.route.end(), next(temp.begin()), temp.end());
    }

    return bus;
}

void Maker::MarkDistance(transport_catalogue::TransportCatalogue& db) {
    Stop* stop_from = nullptr;
    Stop* stop_to = nullptr;
    for (auto& [from, stops] : distance_) {
        stop_from = db.FindStop(from);
        for (auto& [to, distance] : stops.AsDict()) {
            stop_to = db.FindStop(to);
            db.AddDistance(stop_from, stop_to, distance.AsInt());
        }
    }
}

void Maker::MakeRoute(const int id, const std::optional<Route>& route) {
    build_.StartDict();

    build_.Key("request_id").Value(id);

    if (!route.has_value()) {
        build_.Key("error_message").Value("not found");
        build_.EndDict();
        return;
    }
    double time = 0.0;
    build_.Key("items").StartArray();
    for (const auto& item : route.value()) {
        time += item->stat.wait_time + item->stat.travel_time;
        build_.StartDict();
        build_.Key("stop_name").Value(item->from->name);
        build_.Key("time").Value(item->stat.wait_time);
        build_.Key("type").Value("Wait");
        build_.EndDict();
        build_.StartDict();
        build_.Key("bus").Value(item->route->name);
        build_.Key("time").Value(item->stat.travel_time);
        build_.Key("span_count").Value(item->stat.count_stop);
        build_.Key("type").Value("Bus");
        build_.EndDict();
    }
    build_.EndArray();
    build_.Key("total_time").Value(time);

    build_.EndDict();
}

void Maker::MakeBusStat(const int id, const transport_catalogue::StatBuses& buses) const {
    build_.StartDict();

    build_.Key("request_id").Value(id);

    if (buses.empty) {
        build_.Key("error_message").Value("not found");
        build_.EndDict();
        return;
    }

    build_.Key("curvature").Value(buses.curvature);
    build_.Key("route_length").Value(buses.length);
    build_.Key("stop_count").Value(buses.route);
    build_.Key("unique_stop_count").Value(buses.unique);

    build_.EndDict();
}

void Maker::MakeStopStat(const int id, const transport_catalogue::StatStops& stops) const {
    build_.StartDict();

    build_.Key("request_id").Value(id);

    if (stops.empty) {
        build_.Key("error_message").Value("not found");
        build_.EndDict();
        return;
    }

    build_.Key("buses").StartArray();

    for (const auto& bus : stops.name_bus) {
        build_.Value(bus);
    }

    build_.EndArray();

    build_.EndDict();
}

void Maker::MakeMapStat(const int id, const std::string& map) const {
    build_.StartDict();

    build_.Key("request_id").Value(id);

    build_.Key("map").Value(map);

    build_.EndDict();
}

} // namespace detail

void JsonReader::Read(std::istream& input) {
    doc_ = json::Load(input);
}

void JsonReader::StartParse() {
    Parse(doc_.GetRoot());
}

void JsonReader::ReturnStat(std::ostream& output) {
    json::Print(json::Document(build_.Build()), output);
}

void JsonReader::ReturnMap(std::ostream& output) {
    if (!renderer_.ReadyMap()) {
        FillMap();
    }
    svg::Document map;
    RenderMap(map);
    map.Render(output);
}

const transport_router::TransportRouter& JsonReader::GetRouter() const {
    return router_;
}

serialization::serialization_settings JsonReader::ReturnSerializationSettings() {
    const auto& node = doc_.GetRoot();
    if (node.IsDict()) {
        const auto& map = node.AsDict();
        if (map.count("serialization_settings"))
            FillSerializationSettings(map.at("serialization_settings").AsDict());
    }
    return serialization_settings_;
}

void JsonReader::Parse(const json::Node& node) {
    if(node.IsDict()) {
        const auto& map = node.AsDict();
        if (map.count("routing_settings")) {
            FillRouteSettings(map.at("routing_settings").AsDict());
        }
        if (map.count("base_requests")) {
            FillData(map.at("base_requests").AsArray());
        }
        if (map.count("render_settings")) {
            FillMapSettings(map.at("render_settings").AsDict());
        }
        if (map.count("stat_requests")) {
            FillStat(map.at("stat_requests").AsArray());
        }
    }
}

void JsonReader::FillSerializationSettings(const json::Dict& node) {
    serialization_settings_.name_file = node.at("file").AsString();
}

void JsonReader::FillRouteSettings(const json::Dict& node) {
    RoutingSettings routing_settings;

    routing_settings.bus_wait_time = node.at("bus_wait_time").AsInt();
    routing_settings.bus_velocity = node.at("bus_velocity").AsDouble();

    router_.SetSettings(routing_settings);
}

void JsonReader::FillData(const json::Array& node) {
    for (const auto& request : node) {
        if (auto type = request.AsDict().at("type").AsString(); type == "Stop") {
            db_.AddStop(std::move(MakeStop(request.AsDict())));
        } else if (type == "Bus") {
            buslist_.push_back(request);
        }
    }
    for (const auto& request : buslist_) {
        db_.AddBus(std::move(MakeBus(db_, request.AsDict())));
    }
    MarkDistance(db_);
    router_.ConstructRouting();
}

void JsonReader::FillMapSettings(const json::Dict& node) {
    renderer_.AddSettings(std::move(renderer::MapSettings(node)));
}

void JsonReader::FillMap() {
    //renderer_.AddSettings(renderer::MapSettings(node));
    std::vector<::detail::geo::Coordinates> all_coords;
    std::vector<Bus*> buses;
    std::vector<Stop*> stops;
    for (const auto& bus : db_.GetBuses()) {
        buses.emplace_back(db_.FindBus(bus.name));
        for (const auto& stop : bus.route) {
            stops.push_back(stop);
            all_coords.push_back(stop->coordinate);
        }
    }
    std::sort(buses.begin(), buses.end(), [](const Bus* lhs, const Bus* rhs){
        return lhs->name < rhs->name;
    });
    std::sort(stops.begin(), stops.end(), [](const Stop* lhs, const Stop* rhs){
        return lhs->name < rhs->name;
    });
    stops.erase(std::unique(stops.begin(), stops.end()), stops.end());
    renderer_.AddRoute(buses, stops, all_coords);
}

void JsonReader::FillStat(const json::Array& node) {
    build_.StartArray();
    for (const json::Node& request : node) {
        const int id = request.AsDict().at("id").AsInt();
        if (const auto& type = request.AsDict().at("type").AsString(); type == "Stop") {
            const std::string name = request.AsDict().at("name").AsString();
            MakeStopStat(id, GetStopStat(name));
        } else if (type == "Bus") {
            const std::string name = request.AsDict().at("name").AsString();
            MakeBusStat(id, GetBusStat(name));
        } else if (type == "Route") {
            const std::string from = request.AsDict().at("from").AsString();
            const std::string to = request.AsDict().at("to").AsString();
            MakeRoute(id, router_.GetRoute(from, to));
        } else if (type == "Map") {
            if (!renderer_.ReadyMap()) {
                FillMap();
            }
            std::ostringstream out;
            svg::Document map;
            RenderMap(map);
            map.Render(out);
            MakeMapStat(id, out.str());
        }
    }
    build_.EndArray();
}

} // namespace json_reader



