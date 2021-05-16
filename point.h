#pragma once

#ifndef CPPCOURSERA_POINT_H
#define CPPCOURSERA_POINT_H

#endif //CPPCOURSERA_POINT_H

#include <cstdio>
#include <complex>

namespace Points {
#define DEFINE_POINT_STRUCTURE(Name) \
struct Name {                        \
    Name() = default;                \
    explicit Name(double val) : value(val) {} \
    double value;                    \
    operator double() const { return value; } \
};
DEFINE_POINT_STRUCTURE(Latitude)
DEFINE_POINT_STRUCTURE(Longitude)
#undef DEFINE_POINT_STRUCTURE
struct Point {
    Latitude latitude;
    Longitude longitude;
};
    bool operator == (const Point&, const Point&);
    std::ostream& operator << (std::ostream&, const Point&);
    constexpr double earth_radius = 6371 * 1000;
    constexpr double rad_in_degree = 3.1415926535 / 180;
    double CalcLength(const Point& lhs, const Point& rhs);
}