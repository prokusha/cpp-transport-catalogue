#include "json_reader.h"
#include "json.h"
#include "transport_catalogue.h"

#include <iostream>

namespace transport_catalogue {
namespace input {

void JsonReader::Read() {
    json::Document doc = json::Load(input_);
    Parse(doc.GetRoot());
}

json::Node JsonReader::GetRequest() {
    return request_;
}

void JsonReader::Parse(const json::Node& node) {
    auto contex = json::PrintContext{std::cout};
    if(node.IsMap()) {
        for (const auto& base : node.AsMap().at("base_requests").AsArray()) {
            auto type = base.AsMap().at("type").AsString();
            if (type == "Stop") {
                db_.AddStop(std::move(MakeStop(base)));
            } else if (type == "Bus") {
                AddWaitlist(base);
            }
        }
    }
    for (const auto& bus : GetWaitlist()) {
        db_.AddBus(std::move(MakeBus(db_, bus)));
    }
    MakeDistance(db_);
    request_ = node.AsMap().at("stat_requests");
}

} // namespace input
} // namespace transport_catalogue
