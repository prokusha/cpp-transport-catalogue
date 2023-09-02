#include "request_handler.h"
#include "json.h"
#include "map_renderer.h"
#include "svg.h"
#include "transport_catalogue.h"
#include <sstream>

using namespace transport_catalogue;

namespace request {

json::Dict RequestHandler::GetBusStat(const json::Dict& node) const {
    json::Dict stat;

    stat.emplace("request_id", node.at("id").AsInt());

    const auto bus_stat = db_.ReturnStatBus(node.at("name").AsString());
    if (bus_stat.empty) {
        stat.emplace("error_message", std::string("not found"));
        return stat;
    }

    stat.emplace("curvature", bus_stat.curvature);
    stat.emplace("route_length", bus_stat.length);
    stat.emplace("stop_count", bus_stat.route);
    stat.emplace("unique_stop_count", bus_stat.unique);

    return stat;
}

json::Dict RequestHandler::GetStopStat(const json::Dict& node) const {
    json::Dict stat;

    stat.emplace("request_id", node.at("id").AsInt());

    const auto& stat_stop = db_.ReturnStatStop(node.at("name").AsString());
    if (stat_stop.empty) {
        stat.emplace("error_message", std::string("not found"));
        return stat;
    }

    json::Array buses;

    for (const auto& bus : stat_stop.name_bus) {
        buses.push_back(json::Node(bus));
    }

    stat.emplace("buses", buses);

    return stat;
}

json::Dict RequestHandler::GetMapStat(const json::Dict& node) const {
    json::Dict stat;

    stat.emplace("request_id", node.at("id").AsInt());

    std::ostringstream out;
    svg::Document map;
    RenderMap(map);
    map.Render(out);

    stat.emplace("map", out.str());

    return stat;
}

void RequestHandler::RenderMap(svg::Document& map) const {
    renderer_.GetRender(map);
}

} // namespace request

