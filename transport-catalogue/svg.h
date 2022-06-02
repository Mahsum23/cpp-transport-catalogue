#pragma once

#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <optional>
#include <variant>
#include <iomanip>

namespace svg
{
    enum class StrokeLineJoin;
    enum class StrokeLineCap;
    struct Rgba;
    struct Rgb;
    using Color = std::variant<std::monostate, std::string, Rgb, Rgba>;
    inline const std::monostate NoneColor;
}

std::ostream& operator<<(std::ostream& out, const svg::Color& color);

std::ostream& operator<<(std::ostream& out, const svg::StrokeLineCap& slc);

std::ostream& operator<<(std::ostream& out, const svg::StrokeLineJoin& slj);

std::ostream& operator<<(std::ostream& out, const svg::Rgb& color);

std::ostream& operator<<(std::ostream& out, const svg::Rgba& color);

namespace svg
{

    struct Rgb
    {
        Rgb() = default;
        Rgb(int r, int g, int b)
            : red(r), green(g), blue(b)
        {

        }
        uint8_t red = 0;
        uint8_t green = 0;
        uint8_t blue = 0;
    };

    struct Rgba
    {
        Rgba() = default;
        Rgba(int r, int g, int b, double op)
            : red(r), green(g), blue(b), opacity(op)
        {

        }
        uint8_t red = 0;
        uint8_t green = 0;
        uint8_t blue = 0;
        double opacity = 1.0;
    };

    struct ColorPrinter
    {
        std::ostream& out;
        void operator()(std::monostate) const;
        void operator()(std::string color) const;
        void operator()(Rgb color) const;
        void operator()(Rgba color) const;
    };

    enum class StrokeLineCap
    {
        BUTT,
        ROUND,
        SQUARE,
    };

    enum class StrokeLineJoin
    {
        ARCS,
        BEVEL,
        MITER,
        MITER_CLIP,
        ROUND,
    };

    struct Point
    {
        Point() = default;
        Point(double x, double y)
            : x(x)
            , y(y)
        {
        }
        double x = 0;
        double y = 0;
    };

    /*
     * Вспомогательная структура, хранящая контекст для вывода SVG-документа с отступами.
     * Хранит ссылку на поток вывода, текущее значение и шаг отступа при выводе элемента
     */
    struct RenderContext
    {
        RenderContext(std::ostream& out)
            : out(out)
        {
        }

        RenderContext(std::ostream& out, int indent_step, int indent = 0)
            : out(out)
            , indent_step(indent_step)
            , indent(indent)
        {
        }

        RenderContext Indented() const
        {
            return { out, indent_step, indent + indent_step };
        }

        void RenderIndent() const
        {
            for (int i = 0; i < indent; ++i)
            {
                out.put(' ');
            }
        }

        std::ostream& out;
        int indent_step = 0;
        int indent = 0;
    };

    /*
     * Абстрактный базовый класс Object служит для унифицированного хранения
     * конкретных тегов SVG-документа
     * Реализует паттерн "Шаблонный метод" для вывода содержимого тега
     */
    class Object
    {
    public:
        void Render(const RenderContext& context) const;

        virtual ~Object() = default;

    private:
        virtual void RenderObject(const RenderContext& context) const = 0;
    };

    template <typename Owner>
    class PathProps
    {
    public:

        Owner& SetFillColor(Color color)
        {
            fill_color_ = std::move(color);
            return AsOwner();
        }

        Owner& SetStrokeColor(Color color)
        {
            stroke_color_ = std::move(color);
            return AsOwner();
        }

        Owner& SetStrokeWidth(double width)
        {
            stroke_width_ = width;
            return AsOwner();
        }

        Owner& SetStrokeLineCap(StrokeLineCap linecap)
        {
            stroke_linecap_ = linecap;
            return AsOwner();
        }

        Owner& SetStrokeLineJoin(StrokeLineJoin linejoin)
        {
            stroke_linejoin_ = linejoin;
            return AsOwner();
        }
    protected:
        void RenderAttrs(std::ostream& out) const
        {
            using namespace std::literals;

            if (fill_color_)
            {
                out << " fill=\""sv << *fill_color_;
                //std::visit(ColorPrinter{ out }, fill_color_);
                out << "\""sv;
            }
            if (stroke_color_)
            {
                out << " stroke=\""sv << *stroke_color_;
                //std::visit(ColorPrinter{ out }, stroke_color_);
                out << "\""sv;
            }
            if (stroke_width_)
            {
                out << " stroke-width=\""sv << *stroke_width_ << "\""sv;
            }
            if (stroke_linecap_)
            {
                out << " stroke-linecap=\""sv;
                out << *stroke_linecap_ << "\"";
            }
            if (stroke_linejoin_)
            {
                out << " stroke-linejoin=\""sv;
                out << *stroke_linejoin_ << "\"";
            }
        }

    private:
        Owner& AsOwner()
        {
            return static_cast<Owner&>(*this);
        }
        std::optional<Color> fill_color_;
        std::optional<Color> stroke_color_;
        std::optional<double> stroke_width_;
        std::optional<StrokeLineCap> stroke_linecap_;
        std::optional<StrokeLineJoin> stroke_linejoin_;
    };

    class ObjectContainer
    {
    public:
        virtual ~ObjectContainer() = default;
        virtual void AddPtr(std::unique_ptr<Object>&& obj) = 0;
        template <typename Obj>
        void Add(Obj obj)
        {
            AddPtr(std::make_unique<Obj>(obj));
        }
    };

    class Drawable
    {
    public:
        virtual void Draw(ObjectContainer& container) const = 0;
        virtual ~Drawable() = default;
    };


    class Circle final : public Object, public PathProps<Circle>
    {
    public:
        Circle& SetCenter(Point center);
        Circle& SetRadius(double radius);

    private:
        void RenderObject(const RenderContext& context) const override;

        Point center_;
        double radius_ = 1.0;
    };


    class Polyline final : public Object, public PathProps<Polyline>
    {
    public:
        // Добавляет очередную вершину к ломаной линии
        Polyline& AddPoint(Point point);

    private:
        void RenderObject(const RenderContext& context) const override;
        std::vector<Point> points_;
    };


    class Text final : public Object, public PathProps<Text>
    {
    public:
        // Задаёт координаты опорной точки (атрибуты x и y)
        Text& SetPosition(Point pos);

        // Задаёт смещение относительно опорной точки (атрибуты dx, dy)
        Text& SetOffset(Point offset);

        // Задаёт размеры шрифта (атрибут font-size)
        Text& SetFontSize(uint32_t size);

        // Задаёт название шрифта (атрибут font-family)
        Text& SetFontFamily(std::string font_family);

        // Задаёт толщину шрифта (атрибут font-weight)
        Text& SetFontWeight(std::string font_weight);

        // Задаёт текстовое содержимое объекта (отображается внутри тега text)
        Text& SetData(std::string data);
    private:
        void RenderObject(const RenderContext& context) const override;
        Point pos_{ 0,0 };
        Point offset_{ 0,0 };
        uint32_t font_size_ = 1;
        std::string font_family_;
        std::string font_weight_;
        std::string data_ = "";
    };

    class Document : public ObjectContainer
    {
    public:

        // Добавляет в svg-документ объект-наследник svg::Object

        void AddPtr(std::unique_ptr<Object>&& obj);

        // Выводит в ostream svg-представление документа

        void Render(std::ostream& out) const;
    private:
        std::vector<std::unique_ptr<Object>> content_;
    };

}  // namespace svg
