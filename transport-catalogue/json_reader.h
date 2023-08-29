#pragma once

#include "json.h"
#include "transport_catalogue.h"
#include "domain.h"

#include <string>
#include <vector>

namespace transport_catalogue {
namespace input {

class JsonReader : protected detail::Maker {
public:
    JsonReader(transport_catalogue::TransportCatalogue& db, std::istream& input) : db_(db), input_(input) {}
    void Read();
    json::Node GetRequest();

private:
    void Parse(const json::Node& node);

    transport_catalogue::TransportCatalogue& db_;
    std::istream& input_;
    json::Node request_;
};

} // namespace input
} // namespace transport_catalogue


