#pragma once

#include "json.h"
#include "transport_catalogue.h"

#include <optional>
#include <ostream>
#include <unordered_set>

namespace transport_catalogue {
namespace detail {

class RequestHandler {
public:
    RequestHandler(const transport_catalogue::TransportCatalogue& db) : db_(db) {}
    std::optional<transport_catalogue::StatBuses> GetStatusBus(const std::string_view& name);
    std::optional<transport_catalogue::StatStops> GetStatusStop(const std::string_view& name);

private:
    const transport_catalogue::TransportCatalogue& db_;
};

} // namespace detail

namespace output {

class JsonRequest : protected detail::RequestHandler {
public:
    JsonRequest(const transport_catalogue::TransportCatalogue& db, json::Node request, std::ostream& output) : RequestHandler(db), request_(request), output_(output) {}
    void Stats();

private:
    json::Node request_;
    std::ostream& output_;

    json::Dict ReturnStatsBus(const json::Node node);
    json::Dict ReturnStatsStop(const json::Node node);
};

} // namespace output
} // namespace transport_catalogue



