#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <variant>


namespace json
{
    class Node;
}

std::ostream& operator<<(std::ostream& out, const json::Node& node);

namespace json
{
    class Node;

    using Dict = std::map<std::string, Node>;
    using Array = std::vector<Node>;

    class ParsingError : public std::runtime_error
    {
    public:
        using runtime_error::runtime_error;
    };

    class Node
    {
    public:

        using Value = std::variant<std::nullptr_t, int, double, std::string, bool, Array, Dict>;

        Node() = default;
        Node(Array array);
        Node(Dict map);
        Node(bool value);
        Node(double value);
        Node(int value);
        Node(std::string value);
        Node(std::nullptr_t);

        const Value& GetValue() const;

        bool operator==(const Node& other) const;

        bool operator!=(const Node& other) const;

        bool IsInt() const;
        bool IsDouble() const;
        bool IsPureDouble() const;
        bool IsBool() const;
        bool IsString() const;
        bool IsNull() const;
        bool IsArray() const;
        bool IsMap() const;

        int AsInt() const;
        bool AsBool() const;
        double AsDouble() const;
        const std::string& AsString() const;
        const Array& AsArray() const;
        const Dict& AsMap() const;

    private:
        Value value_;
    };

    template <typename Value>
    void PrintValue(const Value& value, std::ostream& out, int indent, int indent_step)
    {
        (void)indent;
        (void)indent_step;
        out << std::boolalpha << value;
    }

    void PrintValue(const std::string& str, std::ostream& out, int indent, int indent_step);

    void PrintValue(std::nullptr_t, std::ostream& out, int indent, int indent_step);

    void PrintValue(const Dict& dict, std::ostream& out, int indent, int indent_step);

    void PrintValue(const Array& arr, std::ostream& out, int indent, int indent_step);

    void PrintNode(const Node& node, std::ostream& out, int indent, int indent_step);

    class Document
    {
    public:
        explicit Document(Node root);

        const Node& GetRoot() const;

        bool operator==(const Document& other) const;
        bool operator!=(const Document& other) const;
    private:
        Node root_;
    };

    Document Load(std::istream& input);

    void Print(const Document& doc, std::ostream& output);

} // namespace json