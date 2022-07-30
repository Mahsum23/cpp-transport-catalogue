#include "svg.h"

std::ostream& operator<<(std::ostream& out, const svg::Color& color)
{
    using namespace std::literals;
    std::visit(svg::ColorPrinter{out}, color);
    return out;
}

std::ostream& operator<<(std::ostream& out, const svg::Rgb& color)
{
    using namespace std::literals;
    out << "rgb("s << int(color.red) << ","s << int(color.green) << ","s << int(color.blue) << ")"s;
    return out;
}

std::ostream& operator<<(std::ostream& out, const svg::Rgba& color)
{
    using namespace std::literals;
    out << "rgba("s << int(color.red) << ","s << int(color.green) << ","s << int(color.blue) << ","s << color.opacity << ")"s;
    return out;
}

std::ostream& operator<<(std::ostream& out, const svg::StrokeLineCap& slc)
{
    using namespace std::literals;
    switch (slc)
    {
        case svg::StrokeLineCap::BUTT:
            out << "butt"sv;
            break;
        case svg::StrokeLineCap::ROUND:
            out << "round"sv;
            break;
        case svg::StrokeLineCap::SQUARE:
            out << "square"sv;
            break;
    }
    return out;
}
    
std::ostream& operator<<(std::ostream& out, const svg::StrokeLineJoin& slj)
{
    using namespace std::literals;
    switch (slj)
    {
        case svg::StrokeLineJoin::ARCS:
            out << "arcs"sv;
            break;
        case svg::StrokeLineJoin::BEVEL:
            out << "bevel"sv;
            break;
        case svg::StrokeLineJoin::MITER:  
            out << "miter"sv;
            break;
        case svg::StrokeLineJoin::MITER_CLIP:
            out << "miter-clip"sv;
            break;
        case svg::StrokeLineJoin::ROUND:
            out << "round"sv;
            break;
    }
    return out;
}

namespace svg {

using namespace std::literals;

void ColorPrinter::operator()(std::monostate) const
{
    out << "none"s;
}

void ColorPrinter::operator()(std::string color) const
{
    out << color;
}

void ColorPrinter::operator()(Rgb color) const
{
    out << color;
}    
    
void ColorPrinter::operator()(Rgba color) const
{
    out << color;
}
    
    
void Object::Render(const RenderContext& context) const {
    context.RenderIndent();

    // Делегируем вывод тега своим подклассам
    RenderObject(context);

    context.out << std::endl;
}

// ---------- Circle ------------------

Circle& Circle::SetCenter(Point center)  {
    center_ = center;
    return *this;
}

Circle& Circle::SetRadius(double radius)  {
    radius_ = radius;
    return *this;
}

void Circle::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
    out << "r=\""sv << radius_ << "\""sv;
    RenderAttrs(out);
    out << "/>"sv;
}
    
// ---------- Polyline ------------------
Polyline& Polyline::AddPoint(Point point)
{
    points_.emplace_back(point);
    return *this;
}

void Polyline::RenderObject(const RenderContext& context) const
{
    auto& out = context.out;
    out << "<polyline points=\"";
    if (!points_.empty())
    {
        out << points_[0].x << "," << points_[0].y;
        for (size_t i = 1; i < points_.size(); ++i)
        {
            out << ' ' << points_[i].x << "," << points_[i].y;
        }
    }
    out << "\""; // end space
    RenderAttrs(out);
    out << "/>"; // no space
}

// ---------- Text ------------------
    // Задаёт координаты опорной точки (атрибуты x и y)
Text& Text::SetPosition(Point pos)
{
    pos_ = pos;
    return *this;
}

// Задаёт смещение относительно опорной точки (атрибуты dx, dy)
Text& Text::SetOffset(Point offset)
{
    offset_ = offset;
    return *this;
}

// Задаёт размеры шрифта (атрибут font-size)
Text& Text::SetFontSize(uint32_t size)
{
    font_size_ = size;
    return *this;
}

// Задаёт название шрифта (атрибут font-family)
Text& Text::SetFontFamily(std::string font_family)
{
    font_family_ = font_family;
    return *this;
}

// Задаёт толщину шрифта (атрибут font-weight)
Text& Text::SetFontWeight(std::string font_weight)
{
    font_weight_ = font_weight;
    return *this;
}

// Задаёт текстовое содержимое объекта (отображается внутри тега text)
Text& Text::SetData(std::string data)
{
    for (char c : data)
    {
        switch (c)
        {
        case '"': 
            data_ += "&quot;";
            break;
        case '\'':
            data_ += "&apos;";
            break;
        case '<':
            data_ += "&lt;";
            break;
        case '>':
            data_ += "&gt;";
            break;
        case '&':
            data_ += "&amp;";
            break;
        default:
            data_.push_back(c);
            break;
        }
    }
    return *this;
}

void Text::RenderObject(const RenderContext& context) const
{
    auto& out = context.out;
    out << "<text";
    RenderAttrs(out);
    out << " x=\"" << pos_.x << "\" " << "y=\"" << pos_.y << "\"";
    
    out << " dx=\"" << offset_.x << "\" " << "dy=\"" << offset_.y << "\"";
    out << " font-size=\"" << font_size_ << "\"";
    if (!font_family_.empty())
        out << " font-family=\"" << font_family_ << "\"";
    if (!font_weight_.empty())
        out << " font-weight=\"" << font_weight_ << "\"";
    out << ">" << data_ << "</text>";
}



void Document::AddPtr(std::unique_ptr<Object>&& obj)
{
    content_.push_back(std::move(obj));
}

void Document::Render(std::ostream& out) const
{
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
    out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;
    RenderContext ctx(out, 2, 2);
    for (auto& obj_ptr : content_)
    {
        obj_ptr->Render(ctx);
    }
    out << "</svg>";
}

}  // namespace svg