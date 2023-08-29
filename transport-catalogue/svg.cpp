#include "svg.h"

#include <memory>
#include <vector>

namespace svg {

using namespace std::literals;

void Object::Render(const RenderContext& context) const {
    context.RenderIndent();

    // Делегируем вывод тега своим подклассам
    RenderObject(context);

    context.out << std::endl;
}

// ---------- Circle ------------------

Circle& Circle::SetCenter(Point center)  {
    center_ = center;
    return *this;
}

Circle& Circle::SetRadius(double radius)  {
    radius_ = radius;
    return *this;
}

void Circle::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
    out << "r=\""sv << radius_ << "\""sv;
    RenderAttrs(out);
    out << "/>"sv;
}

// ----------- Polyline ---------------

Polyline& Polyline::AddPoint(Point point) {
    points_.push_back(point);
    return *this;
}

void Polyline::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<polyline points=\"";

    bool first = true;
    for (auto& point : points_) {
        if (first) {
            out << point.x << ',' << point.y;
            first = false;
            continue;
        }
        out << ' ' << point.x << ',' << point.y;
    }
    out << "\"";
    RenderAttrs(out);
    out << " />";
}

// ------------ Text ------------------

Text& Text::SetPosition(Point point) {
    point_ = point;
    return *this;
}

Text& Text::SetOffset(Point offset) {
    offset_ = offset;
    return *this;
}

Text& Text::SetFontSize(uint32_t size) {
    size_ = size;
    return *this;
}

Text& Text::SetFontFamily(std::string font_famaly) {
    font_family_ = font_famaly;
    return *this;
}

Text& Text::SetFontWeight(std::string font_weight) {
    font_weight_ = font_weight;
    return *this;
}

Text& Text::SetData(std::string data) {
    data_ = Convert(data);
    return *this;
}

std::string Text::Convert(std::string text) {
    std::string edit;
    for (char& c : text) {
        switch (c) {
            case '\"':
                edit += "&quot;";
                break;
            case '<':
                edit += "&lt;";
                break;
            case '>':
                edit += "&rt;";
                break;
            case '\'':
                edit += "&apos;";
                break;
            case '&':
                edit += "&amp;";
                break;
            default:
                edit += c;
        }
    }
    return edit;
}

void Text::RenderObject(const RenderContext& context) const {
    auto& out = context.out;

    out << "<text";
    RenderAttrs(out);
    out << " x=\"" << point_.x <<
    "\" y=\"" << point_.y <<
    "\" dx=\"" << offset_.x <<
    "\" dy=\"" << offset_.y <<
    "\" font-size=\"" << size_;
    if (!font_family_.empty()) {
        out << "\" font-family=\"" << font_family_;
    }

    if (!font_weight_.empty()) {
        out << "\" font-weight=\"" << font_weight_;
    }
    out << "\">" << data_ << "</text>";
}

// ------------ Document --------------

void Document::AddPtr(std::unique_ptr<Object>&& obj) {
    objects_.push_back(std::move(obj));
}

void Document::Render(std::ostream& out) const {

    RenderContext context(out, 2, 2);

    out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>" << '\n';
    out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">" << '\n';

    for (const auto& object : objects_) {
        object->Render(context);
    }

    out << "</svg>";
}

}  // namespace svg
