#pragma once

#include "transport_catalogue.h"
#include "request_handler.h"
#include "json.h"

#include <istream>
#include <ostream>

class JsonReader : protected RequestHandler {
public:
    JsonReader(transport_catalogue::TransportCatalogue db, std::istream& input, std::ostream& output) : RequestHandler(db), input_(input), output_(output) {}
    void Read();

private:
    void Parse(const json::Node& node);

    std::vector<json::Node> waitlist_;

    json::Node Stat;

    std::istream& input_;
    std::ostream& output_;
};
