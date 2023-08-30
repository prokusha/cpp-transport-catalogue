#pragma once

#include "json.h"
#include "transport_catalogue.h"
#include "svg.h"

class Maker {
public:
    void MakeStop(json::Dict jstop);
    void MarkDistance();
    void MakeBus(json::Dict jbus);

private:
    std::vector<std::pair<std::string, json::Node>> distance_;
};

class RequestHandler : protected Maker {
public:
    // , const renderer::MapRenderer& renderer
    RequestHandler(const transport_catalogue::TransportCatalogue& db) : db_(db) {}

    void AddStop(json::Dict jstop);
    void AddBus(json::Dict jbus);
    void AddDistance();

    std::optional<transport_catalogue::StatBuses> GetBusStat(const std::string_view& bus_name) const;

    std::optional<transport_catalogue::StatStops> GetStopStat(const std::string_view& bus_name) const;

    const std::unordered_set<transport_catalogue::Bus*> GetBusesByStop(const std::string_view& stop_name) const;

    svg::Document RenderMap() const;

private:
    const transport_catalogue::TransportCatalogue& db_;
    //const renderer::MapRenderer& renderer_;
};


