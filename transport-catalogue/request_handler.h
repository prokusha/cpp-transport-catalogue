#pragma once

#include "json.h"
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "svg.h"

namespace request {

class RequestHandler {
public:
    RequestHandler(const transport_catalogue::TransportCatalogue& db, const renderer::MapRenderer& renderer) : db_(db), renderer_(renderer) {}

    transport_catalogue::StatBuses GetBusStat(const std::string_view& node) const;

    transport_catalogue::StatStops GetStopStat(const std::string_view& node) const;

    //const std::unordered_set<transport_catalogue::Bus*> GetBusesByStop(const std::string_view& stop_name) const;

    void RenderMap(svg::Document& map) const;

private:
    const transport_catalogue::TransportCatalogue& db_;
    const renderer::MapRenderer& renderer_;
};

} // namespace request



