#pragma once

#include "transport_catalogue.h"
#include "request_handler.h"
#include "json.h"

#include <istream>
#include <ostream>

class Maker {
public:
    transport_catalogue::Stop MakeStop(const json::Dict& jstop);
    void MarkDistance(transport_catalogue::TransportCatalogue& db);
    transport_catalogue::Bus MakeBus(transport_catalogue::TransportCatalogue& db, const json::Dict& jbus);
    json::Dict MakeStatStop(int id, std::optional<transport_catalogue::StatStops> request);
    json::Dict MakeStatBus(int id, std::optional<transport_catalogue::StatBuses> request);

private:
    std::vector<std::pair<std::string, json::Node>> distance_;
};

class JsonReader : protected Maker, protected RequestHandler {
public:
    JsonReader(transport_catalogue::TransportCatalogue& db) : RequestHandler(db), db_(db) {}
    void Read(std::istream& input);
    void ReturnStat(std::ostream& output);
private:
    transport_catalogue::TransportCatalogue& db_;

    std::vector<json::Node> waitlist_;

    json::Array stat_;

    void Parse(const json::Node& node);
    void FillData(const json::Dict& node);
    void FillStat(const json::Dict& node);
};
