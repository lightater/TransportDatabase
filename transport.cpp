#include "transport.h"

#include <algorithm>
#include <iomanip>

namespace Transport {
    BusRoute::BusRoute(Type type, std::vector<StopName> stop_names) : type_(type), stops_names_(std::move(stop_names)) {}
    void BusRoute::Initialize(const std::unordered_map<StopName, StopHandler>& stop_by_name) {
        UpdateStopDistances(stop_by_name);
        info_.num_unique_stops_ = std::set<std::string_view> (stops_names_.cbegin(), stops_names_.cend()).size();
        for (size_t i = 1; i < stops_names_.size(); ++i) {
            StopHandler curr_stop = stop_by_name.at(stops_names_[i]), prev_stop = stop_by_name.at(stops_names_[i - 1]);
            info_.length_ += Points::CalcLength(prev_stop->location,
                                                curr_stop->location);
            info_.road_length_ += prev_stop->distance_to_stops[curr_stop->name];
        }
        switch (type_) {
            case Type::Direct:
                info_.num_stops_ = 2 * stops_names_.size() - 1;
                for (size_t i = stops_names_.size() - 1; i > 0; --i) {
                    StopHandler curr_stop = stop_by_name.at(stops_names_[i]), prev_stop = stop_by_name.at(stops_names_[i - 1]);
                    info_.road_length_ += curr_stop->distance_to_stops[prev_stop->name];
                }
                info_.length_ *= 2;
                break;
            case Type::Circular:
                info_.num_stops_ = stops_names_.size();
                break;
            default:
                throw std::runtime_error("unknown type");
        }
        info_.curvature_ = info_.road_length_ / info_.length_;
    }
    std::ostream& operator << (std::ostream& output, const BusRoute& BusRoute) {
        std::string delimeter = BusRoute.type_ == BusRoute::Type::Direct ? " - " : " > ";
        bool is_first = true;
        for (const auto& stop_name : BusRoute.stops_names_) {
            if (!is_first) output << delimeter;
            is_first = false;
            output << stop_name;
        }
        return output;
    }
    bool BusRoute::operator==(const BusRoute &other) const {
        return std::tie(info_, stops_names_) == std::tie(other.info_, other.stops_names_);
    }
    BusRouteInfo BusRoute::GetInfo() const { return info_; }
    void BusRoute::UpdateStopDistances(const std::unordered_map<StopName, StopHandler>& stop_by_name) {
        for (size_t i = 1; i < stops_names_.size(); ++i) {
            StopHandler curr_stop = stop_by_name.at(stops_names_[i]), prev_stop = stop_by_name.at(stops_names_[i - 1]);
            if (curr_stop->distance_to_stops.count(prev_stop->name) == 0 &&
                prev_stop->distance_to_stops.count(curr_stop->name) == 0) {
                throw std::runtime_error("both distances are empty");
            }
            if (curr_stop->distance_to_stops.count(prev_stop->name) == 0)
                curr_stop->distance_to_stops[prev_stop->name] = prev_stop->distance_to_stops[curr_stop->name];
            if (prev_stop->distance_to_stops.count(curr_stop->name) == 0)
                prev_stop->distance_to_stops[curr_stop->name] = curr_stop->distance_to_stops[prev_stop->name];
        }
    }
    const std::vector<StopName>& BusRoute::GetStopNames() const { return stops_names_; }
    bool operator < (const Stop& lhs, const Stop& rhs) {
        return lhs.name < rhs.name;
    }
    bool operator == (const BusRouteInfo& lhs, const BusRouteInfo& rhs) {
        return std::tie(lhs.num_stops_, lhs.num_unique_stops_, lhs.length_) ==
               std::tie(rhs.num_stops_, rhs.num_unique_stops_, rhs.length_);
    }
    std::ostream& operator << (std::ostream& output, const Stop& stop) {
        return output << stop.name << ' ' << stop.location;
    }
    std::ostream& operator << (std::ostream& output, const BusRoute::Type& type) {
        return output << static_cast<int>(type);
    }
    std::ostream& operator << (std::ostream& output, const BusRouteInfo& BusRoute_info) {
        output << BusRoute_info.num_stops_ << " stops on BusRoute, ";
        output << BusRoute_info.num_unique_stops_ << " unique stops, ";
        output << BusRoute_info.road_length_  << " BusRoute length, ";
        output << std::setprecision(7) << BusRoute_info.curvature_ << " curvature";
        return output;
    }
    std::ostream& operator << (std::ostream& output, const RouteSettings& route_settings) {
        return output << route_settings.bus_wait_time << ' ' << route_settings.bus_velocity;
    }
    bool operator == (const Stop& lhs, const Stop& rhs) {
        return std::tie(lhs.name, lhs.location) == std::tie(rhs.name, rhs.location);
    }
    bool operator == (const Bus& lhs, const Bus& rhs) {
        return std::tie(lhs.number, lhs.route) == std::tie(rhs.number, rhs.route);
    }
    bool operator == (const RouteSettings& lhs, const RouteSettings& rhs) {
        return std::tie(lhs.bus_wait_time, lhs.bus_velocity) == std::tie(rhs.bus_wait_time, rhs.bus_velocity);
    }
    Json::Node NodeFromBus(BusHandler bus, size_t request_id) {
        std::map<std::string, Json::Node> node_map;
        node_map["route_length"] = static_cast<int>(bus->route.GetInfo().road_length_);
        node_map["request_id"] = static_cast<int>(request_id);
        node_map["curvature"] = bus->route.GetInfo().curvature_;
        node_map["stop_count"] = static_cast<int>(bus->route.GetInfo().num_stops_);
        node_map["unique_stop_count"] = static_cast<int>(bus->route.GetInfo().num_unique_stops_);
        return node_map;
    }
    Json::Node NodeFromStop(StopHandler stop, size_t request_id) {
        std::map<std::string, Json::Node> node_map;
        node_map["request_id"] = static_cast<int>(request_id);
        std::vector<Json::Node> buses;
        for (const auto& bus : stop->buses) {
            buses.emplace_back(bus->number);
        }
        std::sort(buses.begin(), buses.end(), [](const Json::Node& lhs, const Json::Node& rhs){
            return lhs.AsString() < rhs.AsString();
        });
        node_map["buses"] = buses;
        return node_map;
    }
    Json::Node NodeFromRouteResponse(const RouteResponse& route_response, size_t request_id) {
        std::map<std::string, Json::Node> result_map;
        result_map["total_time"] = route_response.total_time;
        result_map["request_id"] = static_cast<int>(request_id);
        std::vector<Json::Node> items;
        for (auto action : route_response.actions) {
            std::map<std::string, Json::Node> curr_node_map;
            if (std::holds_alternative<RouteResponse::RouteWaitInfo>(action)) {
                auto action_val = std::get<RouteResponse::RouteWaitInfo>(action);
                curr_node_map["time"] = action_val.time;
                curr_node_map["stop_name"] = action_val.stop_name;
                curr_node_map["type"] = std::string("Wait");
            } else {
                auto action_val = std::get<RouteResponse::RouteBusInfo>(action);
                curr_node_map["span_count"] = static_cast<int>(action_val.span_count);
                curr_node_map["time"] = action_val.time;
                curr_node_map["bus"] = action_val.bus_number;
                curr_node_map["type"] = std::string("Bus");
            }
            items.emplace_back(curr_node_map);
        }
        result_map["items"] = std::move(items);
        return result_map;
    }
}