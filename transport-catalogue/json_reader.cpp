#include "json_reader.h"
#include "geo.h"
#include "json.h"
#include "map_renderer.h"
#include "svg.h"
#include "transport_catalogue.h"

#include <algorithm>
#include <iostream>
#include <sstream>

using namespace transport_catalogue;

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

void Maker::MakeBusStat(const int& id, const transport_catalogue::StatBuses& buses) const {
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

void Maker::MakeStopStat(const int& id, const transport_catalogue::StatStops& stops) const {
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

void Maker::MakeMapStat(const int& id, const std::string& map) const {
    build_.StartDict();

    build_.Key("request_id").Value(id);

    build_.Key("map").Value(map);

    build_.EndDict();
}

} // namespace detail

void JsonReader::Read(std::istream& input) {
    json::Document doc = json::Load(input);
    Parse(doc.GetRoot());
}

void JsonReader::ReturnStat(std::ostream& output) {
    json::Print(json::Document(build_.Build()), output);
}

void JsonReader::ReturnMap(std::ostream& output) {
    svg::Document map;
    RenderMap(map);
    map.Render(output);
}

void JsonReader::Parse(const json::Node& node) {
    if(node.IsDict()) {
        const auto& map = node.AsDict();
        if (map.count("base_requests")) {
            FillData(map.at("base_requests").AsArray());
        }
        if (map.count("render_settings")) {
            FillMap(map.at("render_settings").AsDict());
        }
        if (map.count("stat_requests")) {
            FillStat(map.at("stat_requests").AsArray());
        }
    }
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
}

void JsonReader::FillMap(const json::Dict& node) {
    renderer_.AddSettings(renderer::MapSettings(node));
    std::vector<::detail::geo::Coordinates> all_coords;
    std::vector<Bus*> buses;
    std::vector<Stop*> stops;
    for (const auto& buslist : buslist_) {
        const auto& bus = db_.FindBus(buslist.AsDict().at("name").AsString());
        buses.emplace_back(bus);
        for (const auto& stop : bus->route) {
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
        } else if (type == "Map") {
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



