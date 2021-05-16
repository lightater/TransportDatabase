#include "svg_library.h"
#include <stdexcept>
#include <sstream>

namespace Svg {
    std::ostream& operator << (std::ostream& output, const Color& color) {
        if (std::holds_alternative<std::monostate>(color)) {
            output << "none";
        } else if (std::holds_alternative<std::string>(color)) {
            output << std::get<std::string>(color);
        } else if (std::holds_alternative<Rgb>(color)) {
            const Rgb& rgb = std::get<Rgb>(color);
            output << "rgb(" << static_cast<int>(rgb.red) << ',' << static_cast<int>(rgb.green) << ',' << static_cast<int>(rgb.blue) << ')';
        }
        return output;
    }
    void FigureImpl::SetFillColor(const Color& color) { fill_color_ = color; }
    void FigureImpl::SetStrokeColor(const Color& color) { stroke_color_ = color; }
    void FigureImpl::SetStrokeWidth(double width) { stroke_width_ = width; }
    void FigureImpl::SetStrokeLineCap(const std::string& stroke_linecap) {
        if (stroke_linecap == "butt") {
            stroke_linecap_ = StrokeLineCap::Butt;
        } else if (stroke_linecap == "round") {
            stroke_linecap_ = StrokeLineCap::Round;
        } else if (stroke_linecap == "square") {
            stroke_linecap_ = StrokeLineCap::Square;
        } else {
            throw std::invalid_argument("invalid stroke_line_cap");
        }
    }
    void FigureImpl::SetStrokeLineJoin(const std::string& stroke_linejoin) {
        if (stroke_linejoin == "miter") {
            stroke_linejoin_ = StrokeLineJoin::Miter;
        } else if (stroke_linejoin == "miter-clip") {
            stroke_linejoin_ = StrokeLineJoin::MiterClip;
        } else if (stroke_linejoin == "round") {
            stroke_linejoin_ = StrokeLineJoin::Round;
        } else if (stroke_linejoin == "bevel") {
            stroke_linejoin_ = StrokeLineJoin::Bevel;
        } else if (stroke_linejoin == "arcs") {
            stroke_linejoin_ = StrokeLineJoin::Arcs;
        } else {
            throw std::invalid_argument("invalid stroke_linejoin_");
        }
    }
    std::string FigureImpl::GetDisplay() const {
        std::ostringstream oss;
        oss << "fill=\"" << fill_color_ << "\" stroke=\"" << stroke_color_ << "\" ";
        oss << "stroke-width=\"" << stroke_width_ << "\" ";
        if (stroke_linecap_ != StrokeLineCap::None)
            oss << "stroke-linecap=\"" << stroke_linecap_str_[static_cast<int>(stroke_linecap_) - 1] << "\" ";
        if (stroke_linejoin_ != StrokeLineJoin::None)
            oss << "stroke-linejoin=\"" << stroke_linejoin_str_[static_cast<int>(stroke_linejoin_) - 1] << "\" ";
        return oss.str();
    }
    void CircleImpl::SetCenter(Point center) { center_ = center; }
    void CircleImpl::SetRadius(double radius) { radius_ = radius; }
    std::string CircleImpl::GetDisplay() const {
        std::ostringstream oss;
        oss << "<circle " << FigureImpl::GetDisplay();
        oss << "r=\"" << radius_ << "\" ";
        oss << "cx=\"" << center_.x << "\" cy=\"" << center_.y << "\" ";
        oss << "/>";
        return oss.str();
    }
    void PolylineImpl::AddPoint(Point point) { points_.push_back(point); }
    std::string PolylineImpl::GetDisplay() const {
        std::ostringstream oss;
        oss << "<polyline " << FigureImpl::GetDisplay();
        oss << "points=\"";
        for (const auto& point_ : points_)
            oss << point_.x << ',' << point_.y << ' ';
        oss << "\" />";
        return oss.str();
    }
    void TextImpl::SetPoint(Point point) { point_ = point; }
    void TextImpl::SetOffset(Point offset) { offset_ = offset; }
    void TextImpl::SetFontSize(uint32_t font_size) { font_size_ = font_size; }
    void TextImpl::SetFontFamily(const std::string& font_family) { font_family_ = font_family; }
    void TextImpl::SetData(const std::string& data) { data_ = data; }
    std::string TextImpl::GetDisplay() const {
        std::ostringstream oss;
        oss << "<text " << FigureImpl::GetDisplay();
        oss << "x=\"" << point_.x << "\" y=\"" << point_.y << "\" ";
        oss << "dx=\"" << offset_.x << "\" dy=\"" << offset_.y << "\" ";
        oss << "font-size=\"" << font_size_ << "\" ";
        if (!font_family_.empty()) oss << "font-family=\"" << font_family_ << "\" ";
        oss << ">" << data_ << "</text>";
        return oss.str();
    }

    Circle& Circle::SetCenter(Point center) {
        impl_->SetCenter(center);
        return *this;
    }
    Circle& Circle::SetRadius(double radius) {
        impl_->SetRadius(radius);
        return *this;
    }
    Polyline& Polyline::AddPoint(Point point) {
        impl_->AddPoint(point);
        return *this;
    }
    Text& Text::SetPoint(Point point) {
        impl_->SetPoint(point);
        return *this;
    }
    Text& Text::SetOffset(Point offset) {
        impl_->SetOffset(offset);
        return *this;
    }
    Text& Text::SetFontSize(uint32_t font_size) {
        impl_->SetFontSize(font_size);
        return *this;
    }
    Text& Text::SetFontFamily(const std::string& font_family) {
        impl_->SetFontFamily(font_family);
        return *this;
    }
    Text& Text::SetData(const std::string& data) {
        impl_->SetData(data);
        return *this;
    }
    void Document::Render(std::ostream& output) const {
        output << R"(<?xml version="1.0" encoding="UTF-8" ?>)";
        output << R"(<svg xmlns="http://www.w3.org/2000/svg" version="1.1">)";
        for (auto figure_ : figures_) {
            output << std::visit([](const auto& figure){ return figure.GetDisplay(); },
                                 figure_);
        }
        output << "</svg>\n";
    }
}