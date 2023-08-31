#pragma once

#include "json.h"
#include "transport_catalogue.h"
#include "svg.h"

class RequestHandler {
public:
    // , const renderer::MapRenderer& renderer
    RequestHandler(const transport_catalogue::TransportCatalogue& db) : db_(db) {}

    std::optional<transport_catalogue::StatBuses> GetBusStat(const std::string_view& bus_name) const;

    std::optional<transport_catalogue::StatStops> GetStopStat(const std::string_view& stop_name) const;

    const std::unordered_set<transport_catalogue::Bus*> GetBusesByStop(const std::string_view& stop_name) const;

    svg::Document RenderMap() const;

private:
    const transport_catalogue::TransportCatalogue& db_;
    //const renderer::MapRenderer& renderer_;
};


