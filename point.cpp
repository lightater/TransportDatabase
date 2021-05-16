#include "point.h"
#include <tuple>

namespace Points {
    bool operator == (const Point& lhs, const Point& rhs) {
        return std::tie(lhs.latitude, lhs.longitude) == std::tie(rhs.latitude, rhs.longitude);
    }
    std::ostream& operator << (std::ostream& output, const Point& point) {
        return output << point.latitude << ' ' << point.longitude;
    }
    double CalcLength(const Point& lhs, const Point& rhs) {
        using std::sin, std::cos, std::asin, std::sqrt;
        double sin_la = sin((rhs.latitude - lhs.latitude) * rad_in_degree / 2);
        double sin_lo = sin((rhs.longitude - lhs.longitude) * rad_in_degree / 2);
        return 2 * earth_radius * asin(sqrt(sin_la * sin_la
                                            + sin_lo * sin_lo * cos(rhs.latitude * rad_in_degree)
                                            * cos(lhs.latitude * rad_in_degree)));
    }
}
