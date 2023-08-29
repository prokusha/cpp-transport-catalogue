#include "domain.h"
#include "json.h"

#include <algorithm>

namespace transport_catalogue {
namespace detail {

Stop Maker::MakeStop(const json::Node& stop) {
    Stop stop_;

    stop_.name = stop.AsMap().at("name").AsString();
    stop_.coordinate.lat = stop.AsMap().at("latitude").AsDouble();
    stop_.coordinate.lng = stop.AsMap().at("longitude").AsDouble();
    distance_.push_back({stop_.name, stop.AsMap().at("road_distances")});

    return stop_;
}

Bus Maker::MakeBus(TransportCatalogue& db, const json::Node& bus) {
    Bus bus_;

    bus_.name = bus.AsMap().at("name").AsString();

    for (const auto& stop : bus.AsMap().at("stops").AsArray()) {
        bus_.route.push_back(db.FindStop(stop.AsString()));
    }

    if (!bus.AsMap().at("is_roundtrip").AsBool()) {
        auto temp = bus_.route;
        std::reverse(temp.begin(), temp.end());
        bus_.route.insert(bus_.route.end(), next(temp.begin()), temp.end());
    }

    return bus_;
}

void Maker::MakeDistance(TransportCatalogue& db) {
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

void Maker::AddWaitlist(const json::Node& node) {
    waitlist_.push_back(node);
}

std::vector<json::Node> Maker::GetWaitlist() {
    return waitlist_;
}

} // namespace detail
} // namespace transport_catalogue
