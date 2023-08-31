#include "request_handler.h"

using namespace transport_catalogue;

std::optional<StatBuses> RequestHandler::GetBusStat(const std::string_view& bus_name) const {
    return db_.ReturnStatBus(bus_name);
}

std::optional<StatStops> RequestHandler::GetStopStat(const std::string_view& stop_name) const {
    return db_.ReturnStatStop(stop_name);
}
