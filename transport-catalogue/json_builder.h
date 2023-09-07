#pragma once

#include "json.h"

namespace json {

class KeyItemContext;
class ArrayItemContext;
class DictItemContext;
class ValueItemContext;

class Builder {
public:
    KeyItemContext Key(std::string key);
    Builder& Value(Node value);
    DictItemContext StartDict();
    Builder& EndDict();
    ArrayItemContext StartArray();
    Builder& EndArray();
    Node Build();
private:
    Node root_ = nullptr;
    std::vector<Node*> nodes_stack_;

    template<typename NODE>
    void InsertNode(NODE node, std::string name);
};

class ItemContext {
public:
    ItemContext(Builder& builder) : builder_(builder) {}

    KeyItemContext Key(std::string Key);
    DictItemContext StartDict();
    ArrayItemContext StartArray();

    Builder& Value(Node value);
    Builder& EndDict();
    Builder& EndArray();

private:
    Builder& builder_;
};

class KeyItemContext : public ItemContext {
public:
    KeyItemContext(Builder& builder) : ItemContext(builder) {}
    ValueItemContext Value(Node value);
    KeyItemContext Key(std::string Key) = delete;
    Builder& EndDict() = delete;
    Builder& EndArray() = delete;
};

class ArrayItemContext : public ItemContext {
public:
    ArrayItemContext(Builder& builder) : ItemContext(builder) {}
    ArrayItemContext Value(Node value);
    KeyItemContext Key(std::string Key) = delete;
    Builder& EndDict() = delete;
};

class DictItemContext : public ItemContext {
public:
    DictItemContext(Builder& builder) : ItemContext(builder) {}
    ArrayItemContext StartArray() = delete;
    DictItemContext StartDict() = delete;
    Builder& Value(Node value) = delete;
    Builder& EndArray() = delete;
};

class ValueItemContext : public ItemContext {
public:
    ValueItemContext(Builder& builder) : ItemContext(builder) {}
    Builder& Value(Node value) = delete;
    DictItemContext StartDict() = delete;
    ArrayItemContext StartArray() = delete;
    Builder& EndArray() = delete;
};

template<typename NODE>
void Builder::InsertNode(NODE node, std::string name) {
    if (nodes_stack_.empty()) {
        if (!root_.IsNull()) {
            throw std::logic_error("Obj complite " + name);
        }
        root_ = node;
        nodes_stack_.emplace_back(&root_);
    } else {
        if (nodes_stack_.back()->IsArray()) {
            Node& arr = std::get<Array>(nodes_stack_.back()->GetValue()).emplace_back(node);
            nodes_stack_.emplace_back(&arr);
        } else if (nodes_stack_.back()->IsString()) {
            std::string key = nodes_stack_.back()->AsString();
            nodes_stack_.pop_back();
            Node& node_ = std::get<Dict>(nodes_stack_.back()->GetValue()).at(key);
            node_ = node;
            nodes_stack_.emplace_back(&node_);
        } else {
            throw std::logic_error("Array/Dict");
        }
    }
}

} // namespace json
