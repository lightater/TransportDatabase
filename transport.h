#pragma once

#ifndef CPPCOURSERA_TRANSPORT_H
#define CPPCOURSERA_TRANSPORT_H

#endif //CPPCOURSERA_TRANSPORT_H

#include "point.h"
#include "json.h"
#include <set>
#include <string>
#include <vector>
#include <iterator>
#include <tuple>
#include <unordered_map>
#include <memory>

namespace Transport {
    using BusNumber = std::string;
    using StopName = std::string;
    struct Bus;
    struct Stop;
    using BusHandler = std::shared_ptr<Bus>;
    using StopHandler = std::shared_ptr<Stop>;
    struct Stop {
        std::string name;
        Points::Point location;
        std::unordered_map<StopName, int> distance_to_stops;
        std::set<BusHandler> buses;
    };
    struct BusRouteInfo {
        size_t num_stops_{0}, num_unique_stops_{0};
        double length_{0.}, curvature_{0};
        size_t road_length_{0};
    };
    bool operator < (const Stop& lhs, const Stop& rhs);
    bool operator == (const BusRouteInfo& lhs, const BusRouteInfo& rhs);
    class BusRoute {
    public:
        enum class Type {
            Direct,
            Circular
        } type_;
        BusRoute() = default;
        BusRoute(Type type, std::vector<StopName> stops);
        void Initialize(const std::unordered_map<StopName, StopHandler>& stop_by_name);
        friend std::ostream& operator << (std::ostream& output, const BusRoute& BusRoute);
        bool operator == (const BusRoute& other) const;
        BusRouteInfo GetInfo() const;
        const std::vector<StopName>& GetStopNames() const;
    private:
        std::vector<StopName> stops_names_;
        BusRouteInfo info_;
        void UpdateStopDistances(const std::unordered_map<StopName, StopHandler>& stop_by_name);
    };
    struct Bus {
        BusNumber number;
        BusRoute route;
    };
    struct RouteSettings {
        int bus_wait_time{0};
        double bus_velocity{0.0};
    };
    struct RouteResponse {
        struct RouteWaitInfo {
            double time{0};
            StopName stop_name;
        };
        struct RouteBusInfo {
            size_t span_count{0};
            BusNumber bus_number;
            double time{0};
        };
        using Action = std::variant<RouteWaitInfo, RouteBusInfo>;
        double total_time{0};
        std::vector<Action> actions;
    };
    std::ostream& operator << (std::ostream& output, const Stop& stop);
    std::ostream& operator << (std::ostream& output, const BusRoute::Type& type);
    std::ostream& operator << (std::ostream& output, const BusRouteInfo& BusRoute_info);
    std::ostream& operator << (std::ostream& output, const RouteSettings& route_settings);
    bool operator == (const Stop& lhs, const Stop& rhs);
    bool operator == (const Bus& lhs, const Bus& rhs);
    bool operator == (const RouteSettings& lhs, const RouteSettings& rhs);
    Json::Node NodeFromBus(BusHandler, size_t request_id);
    Json::Node NodeFromStop(StopHandler, size_t request_id);
    Json::Node NodeFromRouteResponse(const RouteResponse&, size_t request_id);
}
