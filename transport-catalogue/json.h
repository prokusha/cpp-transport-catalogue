#pragma once

#include <iostream>
#include <map>
#include <ostream>
#include <string>
#include <vector>
#include <variant>

namespace json {

class Node;
// Сохраните объявления Dict и Array без изменения
using Dict = std::map<std::string, Node>;
using Array = std::vector<Node>;

// Эта ошибка должна выбрасываться при ошибках парсинга JSON
class ParsingError : public std::runtime_error {
public:
    using runtime_error::runtime_error;
};

class Node {
public:

    using Value = std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>;

    Node() = default;
    Node(std::nullptr_t) : Node() {}
    Node(Array array) : value_(std::move(array)) {}
    Node(Dict map) : value_(std::move(map)) {}
    Node(bool value) : value_(value) {}
    Node(int value) : value_(value) {}
    Node(double value) : value_(value) {}
    Node(std::string value) : value_(value) {}

    const Array& AsArray() const;
    const Dict& AsMap() const;
    bool AsBool() const;
    int AsInt() const;
    double AsDouble() const;
    const std::string& AsString() const;

    bool IsInt() const;
    bool IsDouble() const;
    bool IsPureDouble() const;
    bool IsBool() const;
    bool IsString() const;
    bool IsNull() const;
    bool IsArray() const;
    bool IsMap() const;

    const Value& GetValue() const { return value_; }

private:
    Value value_;
};

inline bool operator==(const Node& lhs, const Node& rhs) {
    return lhs.GetValue() == rhs.GetValue();
}

inline bool operator!=(const Node& lhs, const Node& rhs) {
    return !(lhs == rhs);
}

class Document {
public:
    explicit Document(Node root);

    const Node& GetRoot() const;

private:
    Node root_;
};

inline bool operator==(const Document& lhs, const Document& rhs) {
    return lhs.GetRoot() == rhs.GetRoot();
}

inline bool operator!=(const Document& lhs, const Document& rhs) {
    return !(lhs == rhs);
}

Document Load(std::istream& input);

struct PrintContext {
    std::ostream& out;
    int indent_step = 4;
    int indent = 0;

    void PrintIndent() const {
        for (int i = 0; i < indent; ++i) {
            out.put(' ');
        }
    }

    // Возвращает новый контекст вывода с увеличенным смещением
    PrintContext Indented() const {
        return {out, indent_step, indent_step + indent};
    }
};


void PrintNode(const Node& node, const PrintContext& context);

void Print(const Document& doc, std::ostream& output);

struct NodeType {
    const PrintContext& context;
    std::ostream& out = context.out;

    void operator()(std::nullptr_t) const {
        out << "null";
    }

    void operator()(Array array) const {
        out << "[\n";

        auto indent = context.Indented();
        bool first = true;

        for (const auto& node : array) {
            if (first) {
                first = false;
            } else {
                out << ",\n";
            }
            indent.PrintIndent();
            PrintNode(node, indent);
        }
        out << "\n";
        context.PrintIndent();
        out << "]";
    }

    void operator()(Dict dict) const {
        out << "{\n";

        auto indent = context.Indented();
        bool first = true;

        for (const auto& [key, node] : dict) {
            if (first) {
                first = false;
            } else {
                out << ",\n";
            }
            indent.PrintIndent();
            PrintNode(key, indent);
            out << ": ";
            PrintNode(node, indent);
        }
        out << "\n";
        context.PrintIndent();
        out << "}";
    }

    void operator()(bool value) const {
        out << std::boolalpha << value;
    }

    void operator()(std::string value) const {
        out << '"';

        for (const char& c : value) {
            switch (c) {
                case '\n':
                    out << R"(\n)";
                    break;
                case '\r':
                    out << R"(\r)";
                    break;
                case '\"':
                    out << R"(\")";
                    break;
                case '\\':
                    out << R"(\\)";
                    break;
                default:
                    out << c;
                    break;
            }
        }
        out << '"';
    }

    template<typename Value>
    void operator()(Value value) const {
        out << value;
    }
};

}  // namespace json
