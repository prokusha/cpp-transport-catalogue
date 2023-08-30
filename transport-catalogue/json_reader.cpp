#include "json_reader.h"
#include "json.h"

void JsonReader::Read() {
    json::Document doc = json::Load(input_);
    Parse(doc.GetRoot());
}

void JsonReader::Parse(const json::Node& node) {
    if(node.IsMap()) {
        const auto& map = node.AsMap();
        if (map.count("base_requests")) {
            for (const auto& request : map.at("base_requests").AsArray()) {
                if (const auto& type = request.AsMap().at("type").AsString(); type == "Stop") {
                    AddStop(request.AsMap());
                } else if (type == "Bus") {
                    waitlist_.push_back(request);
                }
            }
            for (const auto& request : waitlist_) {
                AddBus(request.AsMap());
            }
            AddDistance();
        }
        if (map.count("render_settings")) {

        }
        if (map.count("stat_requests")) {

        }
    }
}
