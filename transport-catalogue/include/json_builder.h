#pragma once

#include "json.h"

namespace json
{

    class DictItemContext;
    class KeyItemContext;
    class ValueKeyItemContext;
    class ValueItemContext;
    class ArrayItemContext;

    class Builder
    {
    public:
        KeyItemContext Key(std::string key);
        Builder& Value(Node value);
        DictItemContext StartDict();
        ArrayItemContext StartArray();
        Builder& EndDict();
        Builder& EndArray();
        json::Node& Build();
    private:
        json::Node root_;
        std::vector<json::Node*> nodes_stack_{ &root_ };
    };

    class DictItemContext : private Builder
    {
    public:
        DictItemContext(Builder& builder);
        KeyItemContext Key(std::string key);
        Builder& EndDict();
    private:
        Builder& builder_;
    };

    class KeyItemContext : private Builder
    {
    public:
        KeyItemContext(Builder& builder);
        ValueKeyItemContext Value(Node value);
        ArrayItemContext StartArray();
        DictItemContext StartDict();
    private:
        Builder& builder_;
    };

    class ValueKeyItemContext : private Builder
    {
    public:
        ValueKeyItemContext(Builder& builder);
        KeyItemContext Key(std::string key);
        Builder& EndDict();
    private:
        Builder& builder_;
    };

    class ValueItemContext : private Builder
    {
    public:
        ValueItemContext(Builder& builder);
        DictItemContext StartDict();
        ArrayItemContext StartArray();
        Builder& EndArray();
        ValueItemContext Value(Node value);
    private:
        Builder& builder_;
    };

    class ArrayItemContext : private Builder
    {
    public:
        ArrayItemContext(Builder& builder);
        DictItemContext StartDict();
        ArrayItemContext StartArray();
        Builder& EndArray();
        ValueItemContext Value(Node value);
    private:
        Builder& builder_;
    };

}