#include "map_renderer.h"
#include "geo.h"
#include "svg.h"
#include "transport_catalogue.h"

using namespace transport_catalogue;

namespace renderer {

namespace object {

void Route::Draw(svg::ObjectContainer& container) const {
    container.Add(polyline_);
}

svg::Polyline Route::MakeRoute(const Bus* bus, const detail::SphereProjector& proj, MapSettings& settings) {
    svg::Polyline polyline;
    for (const auto& stop : bus->route) {
        polyline.AddPoint(proj(stop->coordinate));
    }
    polyline.SetFillColor("none")
            .SetStrokeColor(settings.color)
            .SetStrokeWidth(settings.line_width)
            .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
            .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
    return polyline;
}

void NameRoute::Draw(svg::ObjectContainer& container) const {
    for (const auto& text : texts_) {
        container.Add(text);
    }
}

std::vector<svg::Text> NameRoute::MakeText(const Bus* bus, const detail::SphereProjector& proj, MapSettings& settings) {
    std::vector<svg::Text> texts;

    svg::Point first_pos = proj(bus->route.front()->coordinate);

    auto bus_label_down = [&bus, &settings](const svg::Point& pos){
        return svg::Text().SetFillColor(settings.underlayer_color)
                          .SetStrokeColor(settings.underlayer_color)
                          .SetStrokeWidth(settings.underlayer_width)
                          .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                          .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
                          .SetPosition(pos)
                          .SetOffset(settings.bus_label_offset)
                          .SetFontSize(settings.bus_label_font_size)
                          .SetFontFamily("Verdana")
                          .SetFontWeight("bold")
                          .SetData(bus->name);
    };

    auto bus_label_top = [&bus, &settings](const svg::Point& pos){
        return svg::Text().SetFillColor(settings.color)
                          .SetPosition(pos)
                          .SetOffset(settings.bus_label_offset)
                          .SetFontSize(settings.bus_label_font_size)
                          .SetFontFamily("Verdana")
                          .SetFontWeight("bold")
                          .SetData(bus->name);
    };

    texts.emplace_back(bus_label_down(first_pos));
    texts.emplace_back(bus_label_top(first_pos));

    if (!bus->is_roundtrip && bus->route[0]->name != bus->route[bus->route.size()/2]->name) {
        svg::Point second_pos = proj(bus->route[bus->route.size()/2]->coordinate);
        texts.emplace_back(bus_label_down(second_pos));
        texts.emplace_back(bus_label_top(second_pos));
    }

    return texts;
}

void DotStop::Draw(svg::ObjectContainer& container) const {
    container.Add(dot_);
}

svg::Circle DotStop::MakeDot(const Stop* stop, const detail::SphereProjector& proj, MapSettings& settings) {
    return svg::Circle().SetCenter(proj(stop->coordinate))
                        .SetRadius(settings.stop_radius)
                        .SetFillColor("white");
}

void NameStops::Draw(svg::ObjectContainer& container) const {
    for (const auto& text : texts_) {
        container.Add(text);
    }
}

std::vector<svg::Text> NameStops::MakeText(const Stop* stop, const detail::SphereProjector& proj, MapSettings& settings) {
    std::vector<svg::Text> texts;

    svg::Point pos = proj(stop->coordinate);

    auto stop_label_down = [&stop, &settings](const svg::Point& pos){
        return svg::Text().SetFillColor(settings.underlayer_color)
                          .SetStrokeColor(settings.underlayer_color)
                          .SetStrokeWidth(settings.underlayer_width)
                          .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                          .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
                          .SetPosition(pos)
                          .SetOffset(settings.stop_label_offset)
                          .SetFontSize(settings.stop_label_font_size)
                          .SetFontFamily("Verdana")
                          .SetData(stop->name);
    };

    auto stop_label_top = [&stop, &settings](const svg::Point& pos){
        return svg::Text().SetFillColor("black")
                          .SetPosition(pos)
                          .SetOffset(settings.stop_label_offset)
                          .SetFontSize(settings.stop_label_font_size)
                          .SetFontFamily("Verdana")
                          .SetData(stop->name);
    };

    texts.emplace_back(stop_label_down(pos));
    texts.emplace_back(stop_label_top(pos));

    return texts;
}

} // namespace object

void MapRenderer::AddSettings(MapSettings settings) {
    settings_ = settings;
    ready_ = false;
}

void MapRenderer::AddRoute(const std::vector<Bus*>& buses, const std::vector<Stop*>& stops, const std::vector<::detail::geo::Coordinates>& coordinates) {
    detail::SphereProjector proj(coordinates.begin(), coordinates.end(), settings_.width, settings_.height, settings_.padding);
    for (const auto& bus : buses) {
        roade_.emplace_back(std::make_unique<object::Route>(bus, proj, settings_));
        name_rouds_.emplace_back(std::make_unique<object::NameRoute>(bus, proj, settings_));
        settings_.NextColor();
    }
    for (const auto& stop : stops) {
        dots_.emplace_back(std::make_unique<object::DotStop>(stop, proj, settings_));
        name_stops_.emplace_back(std::make_unique<object::NameStops>(stop, proj, settings_));
    }
    ready_ = true;
}

void MapRenderer::GetRender(svg::Document& map) const {
    detail::DrawPicture(roade_, map);
    detail::DrawPicture(name_rouds_, map);
    detail::DrawPicture(dots_, map);
    detail::DrawPicture(name_stops_, map);
}

MapSettings MapRenderer::GetSettings() const {
    return settings_;
}

bool MapRenderer::ReadyMap() const {
    return ready_;
}

} // namespace renderer
