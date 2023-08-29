#include "request_handler.h"
#include "json.h"
#include "transport_catalogue.h"

namespace transport_catalogue {
namespace detail {

std::optional<StatBuses> RequestHandler::GetStatusBus(const std::string_view& name) {
    return db_.ReturnStatBus(name);
}
std::optional<transport_catalogue::StatStops> RequestHandler::GetStatusStop(const std::string_view& name) {
    return db_.ReturnStatStop(name);
}

} // namespace detail

namespace output {

void JsonRequest::Stats() {
    json::Array stats;
    for (const auto& request : request_.AsArray()) {
        auto type = request.AsMap().at("type").AsString();
        if (type == "Stop") {
            stats.push_back(std::move(ReturnStatsStop(request)));
        } else if (type == "Bus") {
            stats.push_back(std::move(ReturnStatsBus(request)));
        }
    }
    json::Print(json::Document(json::Node(stats)), output_);
}

json::Dict JsonRequest::ReturnStatsStop(const json::Node node) {
    json::Dict answer;
    answer.emplace("request_id", json::Node(node.AsMap().at("id").AsInt()));
    if (auto stop = GetStatusStop(node.AsMap().at("name").AsString()); stop->empty) {
        std::string error = "not found";
        answer.emplace("error_message", json::Node(std::move(error)));
    } else {
        json::Array buses;
        for (const auto& bus : stop->name_bus) {
            buses.push_back(json::Node(bus));
        }
        answer.emplace("buses", buses);
    }
    return answer;
}

json::Dict JsonRequest::ReturnStatsBus(const json::Node node) {
    json::Dict answer;
    answer.emplace("request_id", json::Node(node.AsMap().at("id").AsInt()));
    if (auto bus = GetStatusBus(node.AsMap().at("name").AsString()); bus->empty) {
        std::string error = "not found";
        answer.emplace("error_message", json::Node(std::move(error)));
    } else {
        answer.emplace("curvature", bus->curvature);
        answer.emplace("route_length", bus->length);
        answer.emplace("stop_count", bus->route);
        answer.emplace("unique_stop_count", bus->unique);
    }
    return answer;
}

} // namespace output
} // namespace transport_catalogue



