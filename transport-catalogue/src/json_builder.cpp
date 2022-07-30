#include "json_builder.h"
#include <variant>


namespace json
{

    DictItemContext Builder::StartDict()
    {
        if (nodes_stack_.empty())
        {
            throw std::logic_error("error starting dict");
        }
        if (root_.IsNull())
        {
            *nodes_stack_.back() = Node(Dict{});
        }
        else if (nodes_stack_.back()->IsArray())
        {
            nodes_stack_.push_back(&std::get<Array>(*nodes_stack_.back()).emplace_back(Node(Dict{})));
        }
        return DictItemContext(*this);
    }

    Builder& Builder::EndDict()
    {
        if (nodes_stack_.empty() || !nodes_stack_.back()->IsDict())
        {
            throw std::logic_error("error ending dict");
        }
        nodes_stack_.pop_back();
        return *this;
    }

    ArrayItemContext Builder::StartArray()
    {
        if (nodes_stack_.empty())
        {
            throw std::logic_error("error starting array");
        }
        if (nodes_stack_.back()->IsArray())
        {
            nodes_stack_.push_back(&std::get<Array>(*nodes_stack_.back()).emplace_back(Array()));
        }
        else
        {
            *nodes_stack_.back() = Node(Array{});
        }
        return *this;
    }

    Builder& Builder::EndArray()
    {
        if (nodes_stack_.empty() || !nodes_stack_.back()->IsArray())
        {
            throw std::logic_error("error ending array");
        }
        nodes_stack_.pop_back();
        return *this;
    }

    KeyItemContext Builder::Key(std::string key)
    {
        if (nodes_stack_.empty() || !nodes_stack_.back()->IsDict())
        {
            throw std::logic_error("key error");
        }
        auto [it, b] = std::get<Dict>(*nodes_stack_.back()).emplace(key, Node(std::string("key")));
        nodes_stack_.push_back(&it->second); // emplace new node to build on stack
        return *this;

    }
    Builder& Builder::Value(Node value)
    {
        if (nodes_stack_.empty() || !(root_.IsNull() || nodes_stack_.back()->IsArray() || (nodes_stack_.back()->IsString() && nodes_stack_.back()->AsString() == "key")))
        {
            throw std::logic_error("value error");
        }
        if (nodes_stack_.back()->IsArray())
        {
            std::get<Array>(*nodes_stack_.back()).emplace_back(value);
        }
        else
        {
            *nodes_stack_.back() = value;
            nodes_stack_.pop_back();
        }
        return *this;
    }

   
    Node& Builder::Build()
    {
        if (!nodes_stack_.empty())
        {
            throw std::logic_error("error");
        }
        return root_;
    }


    // DICT
    DictItemContext::DictItemContext(Builder& builder)
        : builder_(builder)
    {

    }
    KeyItemContext DictItemContext::Key(std::string key)
    {
        return builder_.Key(key);
    }
    Builder& DictItemContext::EndDict()
    {
        return builder_.EndDict();
    }


    // KEY
    KeyItemContext::KeyItemContext(Builder& builder)
        : builder_(builder)
    {

    }
    ValueKeyItemContext KeyItemContext::Value(Node value)
    {
        return builder_.Value(value);
    }
    ArrayItemContext KeyItemContext::StartArray()
    {
        return builder_.StartArray();
    }
    DictItemContext KeyItemContext::StartDict()
    {
        return builder_.StartDict();
    }

    // VALUE AFTER KEY
    ValueKeyItemContext::ValueKeyItemContext(Builder& builder)
        : builder_(builder)
    {

    }
    KeyItemContext ValueKeyItemContext::Key(std::string key)
    {
        return builder_.Key(key);
    }
    Builder& ValueKeyItemContext::EndDict()
    {
        return builder_.EndDict();
    }

    // ARRAY VALUE
    ValueItemContext::ValueItemContext(Builder& builder)
        : builder_(builder)
    {
        
    }
    DictItemContext ValueItemContext::StartDict()
    {
        return builder_.StartDict();
    }
    ArrayItemContext ValueItemContext::StartArray()
    {
        return builder_.StartArray();
    }

    Builder& ValueItemContext::EndArray()
    {
        return builder_.EndArray();
    }
    ValueItemContext ValueItemContext::Value(Node value)
    {
        builder_.Value(value);
        return ValueItemContext(builder_);
    }

    // ARRAY
    ArrayItemContext::ArrayItemContext(Builder& builder)
        : builder_(builder)
    {

    }
    DictItemContext ArrayItemContext::StartDict()
    {
        return builder_.StartDict();
    }
    ArrayItemContext ArrayItemContext::StartArray()
    {
        return builder_.StartArray();
    }
    Builder& ArrayItemContext::EndArray()
    {
        return builder_.EndArray();
    }
    ValueItemContext ArrayItemContext::Value(Node value)
    {
        builder_.Value(value);
        return ValueItemContext(builder_);
    }

}
    
