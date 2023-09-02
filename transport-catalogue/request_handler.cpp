#include "request_handler.h"
#include "map_renderer.h"
#include "svg.h"
#include "transport_catalogue.h"

using namespace transport_catalogue;

namespace request {

StatBuses RequestHandler::GetBusStat(const std::string_view& name) const {
    return db_.ReturnStatBus(name);
}

StatStops RequestHandler::GetStopStat(const std::string_view& name) const {
    return db_.ReturnStatStop(name);
}

void RequestHandler::RenderMap(svg::Document& map) const {
    renderer_.GetRender(map);
}

} // namespace request

