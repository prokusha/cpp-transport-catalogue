#pragma once

#include "graph.h"
#include "json_builder.h"
#include "map_renderer.h"
#include "svg.h"
#include "transport_catalogue.h"
#include "request_handler.h"
#include "json.h"
#include "transport_router.h"
#include "serialization.h"

#include <istream>
#include <ostream>

namespace json_reader {

namespace detail {

class Maker {
public:
    Maker(json::Builder& build) : build_(build) {}
    transport_catalogue::Stop MakeStop(const json::Dict& jstop);
    transport_catalogue::Bus MakeBus(transport_catalogue::TransportCatalogue& db, const json::Dict& jbus);
    void MarkDistance(transport_catalogue::TransportCatalogue& db);
    void MakeRoute(const int id, const std::optional<transport_router::Route>& route);
    void MakeBusStat(const int id, const transport_catalogue::StatBuses& buses) const;
    void MakeStopStat(const int id, const transport_catalogue::StatStops& stops) const;
    void MakeMapStat(const int id, const std::string& map) const;

private:
    std::vector<std::pair<std::string, json::Node>> distance_;
    json::Builder& build_;
    size_t count_stops_ = 0;
    graph::DirectedWeightedGraph<int> graph_;
};

} // namespace detail

class JsonReader : protected detail::Maker, protected request::RequestHandler {
public:
    JsonReader(transport_catalogue::TransportCatalogue& db, renderer::MapRenderer& renderer, transport_router::TransportRouter& router, json::Builder& build) : Maker(build), RequestHandler(db, renderer), db_(db), router_(router), renderer_(renderer), build_(build) {}
    void Read(std::istream& input);
    void StartParse();
    void ReturnStat(std::ostream& output);
    void ReturnMap(std::ostream& output);
    void SetRouterData(serialization::TransportRouterData& RouterData);
    const transport_router::TransportRouter& GetRouter() const;
    serialization::serialization_settings ReturnSerializationSettings();
private:
    transport_catalogue::TransportCatalogue& db_;
    renderer::MapRenderer& renderer_;
    transport_router::TransportRouter& router_;

    std::vector<json::Node> buslist_;

    json::Builder& build_;
    svg::Document map_;

    json::Document doc_;

    serialization::serialization_settings serialization_settings_;

    void Parse(const json::Node& node);
    void FillSerializationSettings(const json::Dict& node);
    void FillRouteSettings(const json::Dict& node);
    void FillData(const json::Array& node);
    void FillMapSettings(const json::Dict& node);
    void FillMap();
    void FillStat(const json::Array& node);
};

} // namespace json_reader
