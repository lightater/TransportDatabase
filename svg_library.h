#pragma once

#ifndef CPPCOURSERA_SVG_LIBRARY_H
#define CPPCOURSERA_SVG_LIBRARY_H

#endif //CPPCOURSERA_SVG_LIBRARY_H

#include <iostream>
#include <memory>
#include <string>
#include <variant>
#include <vector>

namespace Svg {
    struct Point {
        double x{0}, y{0};
    };
    struct Rgb {
        uint8_t red{0}, green{0}, blue{0};
    };
    using Color = std::variant<std::monostate, std::string, Rgb>;
    std::ostream& operator << (std::ostream&, const Color&);
    const Color NoneColor = std::monostate();

    class FigureImpl {
    public:
        void SetFillColor(const Color&);
        void SetStrokeColor(const Color&);
        void SetStrokeWidth(double);
        void SetStrokeLineCap(const std::string&);
        void SetStrokeLineJoin(const std::string&);
        virtual std::string GetDisplay() const = 0;
        virtual ~FigureImpl() = default;
    private:
        Color fill_color_, stroke_color_;
        double stroke_width_{1};
        enum class StrokeLineCap {
            None,
            Butt,
            Round,
            Square
        } stroke_linecap_{StrokeLineCap::None};
        const static inline std::vector<std::string> stroke_linecap_str_ = {"butt", "round", "square"};
        enum class StrokeLineJoin {
            None,
            Miter,
            MiterClip,
            Round,
            Bevel,
            Arcs
        } stroke_linejoin_{StrokeLineJoin::None};
        const static inline std::vector<std::string> stroke_linejoin_str_ = {
                "miter", "miter-clip", "round", "bevel", "arcs"};
    };
    class CircleImpl : public FigureImpl {
    public:
        void SetCenter(Point);
        void SetRadius(double);
        std::string GetDisplay() const override;
    private:
        Point center_;
        double radius_{1};
    };
    class PolylineImpl : public FigureImpl {
    public:
        void AddPoint(Point);
        std::string GetDisplay() const override;
    private:
        std::vector<Point> points_;
    };
    class TextImpl : public FigureImpl {
    public:
        void SetPoint(Point);
        void SetOffset(Point);
        void SetFontSize(uint32_t);
        void SetFontFamily(const std::string&);
        void SetData(const std::string&);
        std::string GetDisplay() const override;
    private:
        Point point_, offset_;
        uint32_t font_size_{1};
        std::string font_family_, data_;
    };

    template <class Fig, class Impl>
    class Figure {
    public:
        Figure() : impl_(std::make_unique<Impl> ()) {}
        Figure(const Figure& other) : impl_(std::make_unique<Impl>(*(other.impl_))) {}
        Figure(Figure&& other) : impl_(std::move(other.impl_)) {}
        Figure& operator = (const Figure& other) { impl_ = std::make_unique<Impl>(*(other.impl_)); return *this; }
        Figure& operator = (Figure&& other) {  impl_ = std::move(other.impl_); return *this; }
        Fig& SetFillColor(const Color& color = NoneColor) {
            impl_->SetFillColor(color);
            return AsFig();
        }
        Fig& SetStrokeColor(const Color& color = NoneColor) {
            impl_->SetStrokeColor(color);
            return AsFig();
        }
        Fig& SetStrokeWidth(double width = 1.0) {
            impl_->SetStrokeWidth(width);
            return AsFig();
        }
        Fig& SetStrokeLineCap(const std::string& stroke_line_cap) {
            impl_->SetStrokeLineCap(stroke_line_cap);
            return AsFig();
        }
        Fig& SetStrokeLineJoin(const std::string& stroke_line_join) {
            impl_->SetStrokeLineJoin(stroke_line_join);
            return AsFig();
        }
        std::string GetDisplay() const {
            return impl_->GetDisplay();
        }
        virtual ~Figure() = default;
    protected:
        std::unique_ptr<Impl> impl_;
    private:
        Fig& AsFig() { return static_cast<Fig&>(*this); }
    };
    class Circle : public Figure<Circle, CircleImpl> {
    public:
        Circle& SetCenter(Point);
        Circle& SetRadius(double);
    };
    class Polyline : public Figure<Polyline, PolylineImpl> {
    public:
        Polyline& AddPoint(Point);
    };
    class Text : public Figure<Text, TextImpl> {
    public:
        Text& SetPoint(Point);
        Text& SetOffset(Point);
        Text& SetFontSize(uint32_t);
        Text& SetFontFamily(const std::string&);
        Text& SetData(const std::string&);
    };
    class Document {
    public:
        template <class Fig>
        void Add(Fig figure) { figures_.push_back(std::move(figure)); }
        void Render(std::ostream&) const;
    private:
        std::vector<std::variant<Circle, Polyline, Text>> figures_;
    };
}