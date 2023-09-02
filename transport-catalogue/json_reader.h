#pragma once

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
    transport_catalogue::Stop MakeStop(const json::Dict& jstop);
    void MarkDistance(transport_catalogue::TransportCatalogue& db);
    transport_catalogue::Bus MakeBus(transport_catalogue::TransportCatalogue& db, const json::Dict& jbus);

private:
    std::vector<std::pair<std::string, json::Node>> distance_;
};

} // namespace detail

class JsonReader : protected detail::Maker, protected request::RequestHandler {
public:
    JsonReader(transport_catalogue::TransportCatalogue& db, renderer::MapRenderer& renderer) : RequestHandler(db, renderer), db_(db), renderer_(renderer) {}
    void Read(std::istream& input);
    void ReturnStat(std::ostream& output);
    void ReturnMap(std::ostream& output);
private:
    transport_catalogue::TransportCatalogue& db_;
    renderer::MapRenderer& renderer_;

    std::vector<json::Node> buslist_;

    json::Array stat_;
    svg::Document map_;

    void Parse(const json::Node& node);
    void FillData(const json::Array& node);
    void FillMap(const json::Dict& node);
    void FillStat(const json::Array& node);
};

} // namespace json_reader
