#pragma once

#include "json.h"
#include "transport_catalogue.h"

namespace transport_catalogue {
namespace detail {

class Maker {
public:
    transport_catalogue::Stop MakeStop(const json::Node& stop);
    transport_catalogue::Bus MakeBus(transport_catalogue::TransportCatalogue& db, const json::Node& bus);
    void MakeDistance(transport_catalogue::TransportCatalogue& db);
    void AddWaitlist(const json::Node& stop);
    std::vector<json::Node> GetWaitlist();

private:
    std::vector<json::Node> waitlist_;
    std::vector<std::pair<std::string, json::Node>> distance_;
};

} // namespace detail
} // namespace transport_catalogue


