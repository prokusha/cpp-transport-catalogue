#pragma once

#include "json_builder.h"
#include "map_renderer.h"
#include "svg.h"
#include "transport_catalogue.h"
#include "request_handler.h"
#include "json.h"

#include <istream>
#include <ostream>

namespace json_reader {

namespace detail {

class Maker {
public:
    Maker(json::Builder& build) : build_(build) {}
    transport_catalogue::Stop MakeStop(const json::Dict& jstop);
    void MarkDistance(transport_catalogue::TransportCatalogue& db);
    transport_catalogue::Bus MakeBus(transport_catalogue::TransportCatalogue& db, const json::Dict& jbus);
    void MakeBusStat(const int& id, const transport_catalogue::StatBuses& buses) const;
    void MakeStopStat(const int& id, const transport_catalogue::StatStops& stops) const;
    void MakeMapStat(const int& id, const std::string& map) const;

private:
    std::vector<std::pair<std::string, json::Node>> distance_;
    json::Builder& build_;
};

} // namespace detail

class JsonReader : protected detail::Maker, protected request::RequestHandler {
public:
    JsonReader(transport_catalogue::TransportCatalogue& db, renderer::MapRenderer& renderer, json::Builder& build) : Maker(build), RequestHandler(db, renderer), db_(db), renderer_(renderer), build_(build) {}
    void Read(std::istream& input);
    void ReturnStat(std::ostream& output);
    void ReturnMap(std::ostream& output);
private:
    transport_catalogue::TransportCatalogue& db_;
    renderer::MapRenderer& renderer_;

    std::vector<json::Node> buslist_;

    json::Builder& build_;
    svg::Document map_;

    void Parse(const json::Node& node);
    void FillData(const json::Array& node);
    void FillMap(const json::Dict& node);
    void FillStat(const json::Array& node);
};

} // namespace json_reader
