#include "json_builder.h"
#include "json.h"

#include <algorithm>
#include <stdexcept>

namespace json {

/*------------BUILDER-----------*/

KeyItemContext Builder::Key(std::string key) {
    if (nodes_stack_.empty() || !nodes_stack_.back()->IsDict()) {
        throw std::logic_error("Key");
    }

    Dict& dict = std::get<Dict>(nodes_stack_.back()->GetValue());
    dict[key] = Node(key);
    nodes_stack_.emplace_back(&dict.at(key));
    return *this;
}

Builder& Builder::Value(Node value) {
    if (nodes_stack_.empty()) {
        if (!root_.IsNull()) {
            throw std::logic_error("Obj complite");
        }
        root_ = std::move(value);
    } else {
        if (nodes_stack_.back()->IsArray()) {
            std::get<Array>(nodes_stack_.back()->GetValue()).emplace_back(std::move(value));
        } else if (nodes_stack_.back()->IsString()) {
            std::string key = nodes_stack_.back()->AsString();
            nodes_stack_.pop_back();
            Node& node_ = std::get<Dict>(nodes_stack_.back()->GetValue()).at(key);
            node_ = std::move(value);
        } else {
            throw std::logic_error("Value");
        }
    }
    return *this;
}

DictItemContext Builder::StartDict() {
    InsertNode(Dict(), "dict");
    return *this;
}

Builder& Builder::EndDict() {
    if (nodes_stack_.empty() || !nodes_stack_.back()->IsDict()) {
        throw std::logic_error("EndDict");
    }
    nodes_stack_.pop_back();
    return *this;
}

ArrayItemContext Builder::StartArray() {
    InsertNode(Array(), "arr");
    return *this;
}

Builder& Builder::EndArray() {
    if (nodes_stack_.empty() || !nodes_stack_.back()->IsArray()) {
        throw std::logic_error("EndArray");
    }
    nodes_stack_.pop_back();
    return *this;
}

Node Builder::Build() {
    if (root_.IsNull() || !nodes_stack_.empty()) {
        throw std::logic_error("Build");
    }
    return root_;
}

/*------------CONTEXT-----------*/

KeyItemContext ItemContext::Key(std::string key) {
    return builder_.Key(std::move(key));
}

DictItemContext ItemContext::StartDict() {
    return builder_.StartDict();
}

ArrayItemContext ItemContext::StartArray() {
    return builder_.StartArray();
}

ArrayItemContext ArrayItemContext::Value(Node value) {
    return ItemContext::Value(std::move(value));
}

Builder& ItemContext::Value(Node value) {
    return builder_.Value(std::move(value));
}

Builder& ItemContext::EndDict() {
    return builder_.EndDict();
}

Builder& ItemContext::EndArray() {
    return builder_.EndArray();
}

DictItemContext KeyItemContext::Value(Node value) {
    return ItemContext::Value(std::move(value));
}

} // namespace json
