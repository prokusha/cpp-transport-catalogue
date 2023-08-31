#include "json_reader.h"
#include "json.h"
#include "transport_catalogue.h"

#include <algorithm>

using namespace transport_catalogue;

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

    if (!jbus.at("is_roundtrip").AsBool()) {
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
        for (auto& [to, distance] : stops.AsMap()) {
            stop_to = db.FindStop(to);
            db.AddDistance(stop_from, stop_to, distance.AsInt());
        }
    }
}

json::Dict Maker::MakeStatStop(int id, std::optional<StatStops> request) {
    json::Dict stat;

    stat.emplace("request_id", id);

    if (request.value().empty) {
        stat.emplace("error_message", std::string("not found"));
        return stat;
    }

    json::Array buses;

    for (const auto& bus : request.value().name_bus) {
        buses.push_back(json::Node(bus));
    }

    stat.emplace("buses", buses);

    return stat;
}

json::Dict Maker::MakeStatBus(int id, std::optional<StatBuses> request) {
    json::Dict stat;

    stat.emplace("request_id", id);

    if (request.value().empty) {
        stat.emplace("error_message", std::string("not found"));
        return stat;
    }

    stat.emplace("curvature", request.value().curvature);
    stat.emplace("route_length", request.value().length);
    stat.emplace("stop_count", request.value().route);
    stat.emplace("unique_stop_count", request.value().unique);

    return stat;
}

void JsonReader::Read(std::istream& input) {
    json::Document doc = json::Load(input);
    Parse(doc.GetRoot());
}

void JsonReader::ReturnStat(std::ostream& output) {
    json::Print(json::Document(json::Node(stat_)), output);
}

void JsonReader::Parse(const json::Node& node) {
    if(node.IsMap()) {
        const auto& map = node.AsMap();
        if (map.count("base_requests")) {
            FillData(map);
        }
        if (map.count("stat_requests")) {
            FillStat(map);
        }
    }
}

void JsonReader::FillData(const json::Dict& node) {
    for (const auto& request : node.at("base_requests").AsArray()) {
        if (auto type = request.AsMap().at("type").AsString(); type == "Stop") {
            db_.AddStop(std::move(MakeStop(request.AsMap())));
        } else if (type == "Bus") {
            waitlist_.push_back(request);
        }
    }
    for (const auto& request : waitlist_) {
        db_.AddBus(std::move(MakeBus(db_, request.AsMap())));
    }
    MarkDistance(db_);
}

void JsonReader::FillStat(const json::Dict& node) {
    for (const json::Node& request : node.at("stat_requests").AsArray()) {
        int id = request.AsMap().at("id").AsInt();
        std::string name = request.AsMap().at("name").AsString();
        if (const auto& type = request.AsMap().at("type").AsString(); type == "Stop") {
            stat_.push_back(std::move(MakeStatStop(id, GetStopStat(name))));
        } else if (type == "Bus") {
            stat_.push_back(std::move(MakeStatBus(id, GetBusStat(name))));
        }
    }
}
