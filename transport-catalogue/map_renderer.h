#pragma once

#include "geo.h"
#include "json.h"
#include "svg.h"
#include "transport_catalogue.h"

#include <algorithm>

namespace renderer {

namespace detail {

class SphereProjector {
public:
    template <typename PointInputIt>
    SphereProjector(PointInputIt points_begin, PointInputIt points_end,
                    double max_width, double max_height, double padding)
        : padding_(padding) //
    {
        // Если точки поверхности сферы не заданы, вычислять нечего
        if (points_begin == points_end) {
            return;
        }

        // Находим точки с минимальной и максимальной долготой
        const auto [left_it, right_it] = std::minmax_element(
            points_begin, points_end,
            [](auto lhs, auto rhs) { return lhs.lng < rhs.lng; });
        min_lon_ = left_it->lng;
        const double max_lon = right_it->lng;

        // Находим точки с минимальной и максимальной широтой
        const auto [bottom_it, top_it] = std::minmax_element(
            points_begin, points_end,
            [](auto lhs, auto rhs) { return lhs.lat < rhs.lat; });
        const double min_lat = bottom_it->lat;
        max_lat_ = top_it->lat;

        // Вычисляем коэффициент масштабирования вдоль координаты x
        std::optional<double> width_zoom;
        if (!IsZero(max_lon - min_lon_)) {
            width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
        }

        // Вычисляем коэффициент масштабирования вдоль координаты y
        std::optional<double> height_zoom;
        if (!IsZero(max_lat_ - min_lat)) {
            height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
        }

        if (width_zoom && height_zoom) {
            // Коэффициенты масштабирования по ширине и высоте ненулевые,
            // берём минимальный из них
            zoom_coeff_ = std::min(*width_zoom, *height_zoom);
        } else if (width_zoom) {
            // Коэффициент масштабирования по ширине ненулевой, используем его
            zoom_coeff_ = *width_zoom;
        } else if (height_zoom) {
            // Коэффициент масштабирования по высоте ненулевой, используем его
            zoom_coeff_ = *height_zoom;
        }
    }

    // Проецирует широту и долготу в координаты внутри SVG-изображения
    svg::Point operator()(transport_catalogue::detail::geo::Coordinates coords) const {
        return {
            (coords.lng - min_lon_) * zoom_coeff_ + padding_,
            (max_lat_ - coords.lat) * zoom_coeff_ + padding_
        };
    }

private:
    double padding_;
    double min_lon_ = 0;
    double max_lat_ = 0;
    double zoom_coeff_ = 0;

    const double EPSILON = 1e-6;

    bool IsZero(double value) {
        return std::abs(value) < EPSILON;
    }
};

template <typename DrawableIterator>
void DrawPicture(DrawableIterator begin, DrawableIterator end, svg::ObjectContainer& target) {
    for (auto it = begin; it != end; ++it) {
        (*it)->Draw(target);
    }
}

template <typename Container>
void DrawPicture(const Container& container, svg::ObjectContainer& target) {
    using namespace std;
    DrawPicture(begin(container), end(container), target);
}

} // namespace detail

struct MapSettings;

namespace object {

class Route : public svg::Drawable {
public:
    Route(const transport_catalogue::Bus* bus, const detail::SphereProjector& proj, MapSettings& settings) : polyline_(MakeRoute(bus, proj, settings)) {}
    void Draw(svg::ObjectContainer& container) const override;
private:
    svg::Polyline polyline_;
    svg::Polyline MakeRoute(const transport_catalogue::Bus* bus, const detail::SphereProjector& proj, MapSettings& settings);
};

class NameRoute : public svg::Drawable {
public:
    NameRoute(const transport_catalogue::Bus* bus, const detail::SphereProjector& proj, MapSettings& settings) : texts_(MakeText(bus, proj, settings)) {}
    void Draw(svg::ObjectContainer& container) const override;
private:
    std::vector<svg::Text> texts_;
    std::vector<svg::Text> MakeText(const transport_catalogue::Bus* bus, const detail::SphereProjector& proj, MapSettings& settings);
};

class DotStop : public svg::Drawable {
public:
    DotStop(const transport_catalogue::Stop* stop, const detail::SphereProjector& proj, MapSettings& settings) : dot_(MakeDot(stop, proj, settings)) {}
    void Draw(svg::ObjectContainer& container) const override;
private:
    svg::Circle dot_;
    svg::Circle MakeDot(const transport_catalogue::Stop* stop, const detail::SphereProjector& proj, MapSettings& settings);
};

class NameStops : public svg::Drawable {
public:
    NameStops(const transport_catalogue::Stop* stop, const detail::SphereProjector& proj, MapSettings& settings) : texts_(MakeText(stop, proj, settings)) {}
    void Draw(svg::ObjectContainer& container) const override;
private:
    std::vector<svg::Text> texts_;
    std::vector<svg::Text> MakeText(const transport_catalogue::Stop* stop, const detail::SphereProjector& proj, MapSettings& settings);
};

} // namespace object

struct MapSettings {
public:
    MapSettings() = default;
    MapSettings(const json::Dict& settings) {
        width = settings.at("width").AsDouble();
        height = settings.at("height").AsDouble();
        padding = settings.at("padding").AsDouble();
        line_width = settings.at("line_width").AsDouble();
        stop_radius = settings.at("stop_radius").AsDouble();
        underlayer_width = settings.at("underlayer_width").AsDouble();

        bus_label_font_size = settings.at("bus_label_font_size").AsInt();

        stop_label_font_size = settings.at("stop_label_font_size").AsInt();

        bus_label_offset.x = settings.at("bus_label_offset").AsArray().front().AsDouble();
        bus_label_offset.y = settings.at("bus_label_offset").AsArray().back().AsDouble();

        stop_label_offset.x = settings.at("stop_label_offset").AsArray().front().AsDouble();
        stop_label_offset.y = settings.at("stop_label_offset").AsArray().back().AsDouble();

        underlayer_color = ReturnColor(settings.at("underlayer_color"));

        for (const auto& node : settings.at("color_palette").AsArray()) {
            color_palette.push_back(ReturnColor(node));
        }

        color = color_palette[it];
    }

    void NextColor() {
        if (it == color_palette.size() - 1) {
            it = 0;
        } else {
            ++it;
        }
        color = color_palette[it];
    }

    double width;
    double height;
    double padding;
    double line_width;
    double stop_radius;
    int bus_label_font_size;
    svg::Point bus_label_offset;
    int stop_label_font_size;
    svg::Point stop_label_offset;
    svg::Color underlayer_color;
    double underlayer_width;
    std::vector<svg::Color> color_palette;
    svg::Color color;

private:
    std::vector<svg::Color>::size_type it = 0;

    svg::Color ReturnColor(const json::Node& node) {
        if (node.IsArray()) {
            if (const auto& arr = node.AsArray(); arr.size() == 3) {
                return svg::Rgb(arr[0].AsInt(), arr[1].AsInt(), arr[2].AsInt());
            } else if (arr.size() == 4) {
                return svg::Rgba(arr[0].AsInt(), arr[1].AsInt(), arr[2].AsInt(), arr[3].AsDouble());
            }
        }
        return node.AsString();
    }
};

class MapRenderer {
public:
    void AddSettings(MapSettings settings);
    void AddRoute(const std::vector<transport_catalogue::Bus*>& buses, const std::vector<transport_catalogue::Stop*>& stops, const std::vector<transport_catalogue::detail::geo::Coordinates>& coordinates);
    void GetRender(svg::Document& map) const;

private:
    std::vector<std::unique_ptr<svg::Drawable>> roade_;
    std::vector<std::unique_ptr<svg::Drawable>> name_rouds_;
    std::vector<std::unique_ptr<svg::Drawable>> dots_;
    std::vector<std::unique_ptr<svg::Drawable>> name_stops_;
    MapSettings settings_;
};


} // namespace renderer
