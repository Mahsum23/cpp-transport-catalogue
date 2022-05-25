#include "json.h"

using namespace std;

std::ostream& operator<<(std::ostream& out, const json::Node& node)
{
    json::PrintNode(node, out, 0, 4);
    return out;
}

namespace json {

    namespace {

        Node LoadNode(istream& input);

        Node LoadArray(istream& input) 
        {
            Array result;
            char c;
            for (; input >> c && c != ']';) 
            {
                if (c != ',')
                {
                    input.putback(c);
                }
                result.push_back(LoadNode(input));
            }
            if (c != ']')
            {
                throw ParsingError("error parsing array");
            }
            return Node(move(result));
        }

        using Number = std::variant<int, double>;

        Node LoadNumber(std::istream& input) 
        {
            using namespace std::literals;

            std::string parsed_num;

            auto read_char = [&parsed_num, &input] 
            {
                parsed_num += static_cast<char>(input.get());
                if (!input)
                {
                    throw ParsingError("Failed to read number from stream"s);
                }
            };

            auto read_digits = [&input, read_char] 
            {
                if (!std::isdigit(input.peek())) 
                {
                    throw ParsingError("A digit is expected"s);
                }
                while (std::isdigit(input.peek()))
                {
                    read_char();
                }
            };

            if (input.peek() == '-')
            {
                read_char();
            }
            if (input.peek() == '0') 
            {
                read_char();
            }
            else 
            {
                read_digits();
            }

            bool is_int = true;
            if (input.peek() == '.')
            {
                read_char();
                read_digits();
                is_int = false;
            }

            if (int ch = input.peek(); ch == 'e' || ch == 'E')
            {
                read_char();
                if (ch = input.peek(); ch == '+' || ch == '-') 
                {
                    read_char();
                }
                read_digits();
                is_int = false;
            }

            try 
            {
                if (is_int) 
                {
                    try 
                    {
                        return Node(std::stoi(parsed_num));
                    }
                    catch (...) 
                    {
                    }
                }
                return Node(std::stod(parsed_num));
            }
            catch (...) 
            {
                throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
            }
        }

        Node LoadString(std::istream& input) 
        {
            using namespace std::literals;

            auto it = std::istreambuf_iterator<char>(input);
            auto end = std::istreambuf_iterator<char>();
            std::string s;
            while (true) 
            {
                if (it == end) 
                {
                    throw ParsingError("String parsing error");
                }
                const char ch = *it;
                if (ch == '"') 
                {
                    ++it;
                    break;
                }
                else if (ch == '\\')
                {
                    ++it;
                    if (it == end)
                    {
                        throw ParsingError("String parsing error");
                    }
                    const char escaped_char = *(it);
                    switch (escaped_char) 
                    {
                    case 'n':
                        s.push_back('\n');
                        break;
                    case 't':
                        s.push_back('\t');
                        break;
                    case 'r':
                        s.push_back('\r');
                        break;
                    case '"':
                        s.push_back('"');
                        break;
                    case '\\':
                        s.push_back('\\');
                        break;
                    default:
                        throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
                    }
                }
                else if (ch == '\n' || ch == '\r')
                {
                    throw ParsingError("Unexpected end of line"s);
                }
                else 
                {
                    s.push_back(ch);
                }
                ++it;
            }

            return Node(move(s));
        }

        Node LoadDict(istream& input)
        {
            Dict result;
            char c;
            for (; input >> c && c != '}';)
            {
                if (c == ',')
                {
                    input >> c;
                }

                string key = LoadString(input).AsString();
                input >> c;
                result.insert({ move(key), LoadNode(input) });
            }
            if (c != '}')
            {
                throw ParsingError("unexpected symbol: }");
            }
            return Node(move(result));
        }

        Node LoadNull(istream& input)
        {
            string ull = "ull";
            for (int i = 0; i < 3; ++i)
            {
                if (input.peek() != ull[i])
                {

                    throw ParsingError("incorrect null");
                }
                input.get();
            }
            return Node();
        }

        Node LoadNode(istream& input) 
        {
            char c;
            input >> c;
            if ((c == ']') || (c == '}'))
            {
                throw ParsingError("unexpected symbol: " + string(c, 1));
            }
            if (c == '[')
            {
                return LoadArray(input);
            }
            else if (c == '{')
            {
                return LoadDict(input);
            }
            else if (c == '"')
            {
                return LoadString(input);
            }
            else if (c == 'n')
            {
                return LoadNull(input);
            }
            else if (c == 'f')
            {
                string s = "alse";
                for (int i = 0; i < 4; ++i)
                {
                    if (input.peek() != s[i])
                    {
                        throw ParsingError("error reading false");
                    }
                    input.get();
                }
                return Node(false);
            }
            else if (c == 't')
            {
                string s = "rue";
                for (int i = 0; i < 3; ++i)
                {
                    if (input.peek() != s[i])
                    {
                        throw ParsingError("error reading true");
                    }
                    input.get();
                }
                return Node(true);
            }
            else
            {
                input.putback(c);
                return LoadNumber(input);
            }

        }

    }  // namespace

    Node::Node(Array array)
        : value_(move(array)) 
    {
    }

    Node::Node(Dict map)
        : value_(move(map))
    {
    }

    Node::Node(int value)
        : value_(value) 
    {
    }

    Node::Node(double value)
        : value_(value) 
    {
    }

    Node::Node(string value)
        : value_(move(value)) 
    {
    }

    Node::Node(nullptr_t)
    {
    }

    Node::Node(bool value)
        : value_(value)
    {
    }

    const Node::Value& Node::GetValue() const
    {
        return value_;
    }

    bool Node::operator==(const Node& other) const
    {
        return GetValue() == other.GetValue();
    }

    bool Node::operator!=(const Node& other) const
    {
        return !(*this == other);
    }

    bool Node::IsInt() const
    {
        return holds_alternative<int>(value_);
    }

    bool Node::IsDouble() const
    {
        return holds_alternative<double>(value_) || holds_alternative<int>(value_);
    }

    bool Node::IsPureDouble() const
    {
        return holds_alternative<double>(value_);
    }

    bool Node::IsBool() const
    {
        return holds_alternative<bool>(value_);
    }

    bool Node::IsString() const
    {
        return holds_alternative<std::string>(value_);
    }

    bool Node::IsNull() const
    {
        return holds_alternative<std::nullptr_t>(value_);
    }

    bool Node::IsArray() const
    {
        return holds_alternative<Array>(value_);
    }
    bool Node::IsMap() const
    {
        return holds_alternative<Dict>(value_);
    }

    const Array& Node::AsArray() const {
        if (!IsArray())
        {
            throw std::logic_error("not array");
        }
        return get<Array>(value_);
    }

    double Node::AsDouble() const {
        if (!IsDouble())
        {
            throw std::logic_error("not double");  
        }
        if (IsInt())
        {
            return get<int>(value_);
        }
        return get<double>(value_);
    }

    const Dict& Node::AsMap() const {
        if (!IsMap())
        {
            throw std::logic_error("not map");
        }
        return get<Dict>(value_);
    }

    int Node::AsInt() const {
        if (!IsInt())
        {
            throw std::logic_error("not int");
        }
        return get<int>(value_);
    }

    bool Node::AsBool() const {
        if (!IsBool())
        {
            throw std::logic_error("not bool");
        }
        return get<bool>(value_);
    }

    const string& Node::AsString() const {
        if (!IsString())
        {
            throw std::logic_error("not string");
        }
        return get<std::string>(value_);
    }

    void PrintValue(const Array& arr, std::ostream& out, int indent, int indent_step)
    {
        using namespace std::literals;
        out << "["sv << '\n';
        if (arr.empty())
        {
            out << "]"sv;
            return;
        }
        out << std::string(indent+indent_step, ' ');
        PrintNode(arr[0], out, indent+indent_step, indent_step);
        for (size_t i = 1; i < arr.size(); ++i)
        {
            out << ',' << '\n' << std::string(indent+indent_step, ' ');
            PrintNode(arr[i], out, indent+indent_step, indent_step);
        }
        out << "\n" << std::string(indent, ' ') << "]"sv; // changed
    }

    void PrintValue(const Dict& dict, std::ostream& out, int indent, int indent_step)
    {
        using namespace std::literals;

        out << '{' << '\n';
        out << std::string(indent+indent_step, ' ');
        out << '\"' << dict.begin()->first << "\": "sv;
        PrintNode(dict.begin()->second, out, indent+indent_step, indent_step);
        for (auto it = next(dict.begin()); it != dict.end(); it = next(it))
        {
            out << ',' << '\n'  << std::string(indent+indent_step, ' ') << '\"' << it->first << "\": "sv;
            PrintNode(it->second, out, indent, indent_step);
        }
        out << '\n' << std::string(indent, ' ') << '}';
    }

    void PrintValue(std::nullptr_t, std::ostream& out, int indent, int indent_step)
    {
        out << std::string(indent, ' ');
        out << "null"sv;
    }

    void PrintValue(const std::string& str, std::ostream& out, int indent, int indent_step)
    {
        (void)indent;
        (void)indent_step;
        using namespace std::literals;
        out << "\"";
        for (char c : str)
        {
            switch (c)
            {
            case '\t':
                out << "\t"sv;
                break;
            case '\n':
                out << "\\n"sv;
                break;
            case '\"':
                out << "\\\""sv;
                break;
            case '\r':
                out << "\\r"sv;
                break;
            case '\\':
                out << "\\\\"sv;
                break;
            default:
                out << c;
            }
        }
        out << "\"";
    }

    void PrintNode(const Node& node, std::ostream& out, int indent, int indent_step)
    {
        std::visit([&out, indent, indent_step](const auto& value) { PrintValue(value, out, indent, indent_step); },
            node.GetValue());
    }

    Document::Document(Node root)
        : root_(move(root)) 
    {
    }

    const Node& Document::GetRoot() const 
    {
        return root_;
    }

    Document Load(istream& input) {
        return Document{ LoadNode(input) };
    }
    
    bool Document::operator==(const Document& other) const
    {
        return root_ == other.root_;
    }
    bool Document::operator!=(const Document& other) const
    {
        return !(*this == other);
    }

    void Print(const Document& doc, std::ostream& output) 
    {
        PrintNode(doc.GetRoot(), output, 0, 2);
    }

}  // namespace json