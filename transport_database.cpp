#include "transport_database.h"
#include "utils.h"

namespace Transport {
    void TransportDatabase::AddRoutingSettings(RouteSettings route_settings) { route_settings_ = route_settings; }
    void TransportDatabase::AddStop(StopHandler stop) { stop_by_name_[stop->name] = stop; }
    void TransportDatabase::AddBus(BusHandler bus) {
        bus->route.Initialize(stop_by_name_);
        bus_by_number_[bus->number] = bus;
        for (const auto& stop_name : bus->route.GetStopNames()) {
            stop_by_name_[stop_name]->buses.insert(bus);
        }
    }
    Json::Node TransportDatabase::GetBus(const BusNumber& number, size_t request_id) const {
        if (bus_by_number_.count(number) == 0) return NotFound(request_id);
        return NodeFromBus(bus_by_number_.at(number), request_id);
    }
    Json::Node TransportDatabase::GetStop(const StopName& name, size_t request_id) const {
        if (stop_by_name_.count(name) == 0) return NotFound(request_id);
        return NodeFromStop(stop_by_name_.at(name), request_id);
    }
    Json::Node TransportDatabase::GetRoute(const StopName& from, const StopName& to, size_t request_id) const {
        std::optional<RouteResponse> route_response = BuildRoute(from, to);
        if (!route_response) return NotFound(request_id);
        return NodeFromRouteResponse(*route_response, request_id);
    }
    void TransportDatabase::InitializeGraph() {
        size_t vertex_count = stop_by_name_.size();
        for (const auto& [_, bus] : bus_by_number_) {
            size_t add = bus->route.GetStopNames().size();
            vertex_count += bus->route.type_ == BusRoute::Type::Direct ? add * 2 : add;
        }
        graph_ = std::make_unique<Graph::DirectedWeightedGraph<double>>(vertex_count);
    }
    void TransportDatabase::InitializeRouter() {
        InitializeGraph();
        std::vector<Graph::VertexId> abstract_vertexes;
        size_t vertex_count = 0;
        for (const auto& [stop_name, _] : stop_by_name_) {
            abstract_id_by_name_[stop_name] = vertex_count;
            abstract_vertexes.push_back(vertex_count);
            vertex_by_id_[vertex_count++] = {stop_name, std::nullopt};
        }
        for (const auto& [bus_number, bus] : bus_by_number_) {
            const std::vector<StopName>& stop_names = bus->route.GetStopNames();
            std::vector<StopHandler> stops;
            for (const auto& stop_name : stop_names) stops.push_back(stop_by_name_.at(stop_name));
            AddBusRouteToGraph(stops.cbegin(), stops.cend(), vertex_count, bus_number);
            if (bus->route.type_ == BusRoute::Type::Direct)
                AddBusRouteToGraph(stops.crbegin(), stops.crend(), vertex_count, bus_number);
        }
        router_ = std::make_unique<Graph::Router<double>>(*graph_, abstract_vertexes);
    }
    Json::Node TransportDatabase::NotFound(size_t request_id) {
        return std::map<std::string, Json::Node> {{"request_id", static_cast<int>(request_id)},
                                                  {"error_message", std::string("not found")}};
    }
    std::optional<RouteResponse> TransportDatabase::BuildRoute(const StopName& from, const StopName& to) const {
        std::optional<Graph::Router<double>::RouteInfo> route_info = router_->BuildRoute(
                abstract_id_by_name_.at(from), abstract_id_by_name_.at(to)
                );
        if (!route_info) return std::nullopt;
        RouteResponse result;
        RouteResponse::Action curr_action = RouteResponse::RouteWaitInfo{0, ""};
        for (auto edge_id = 0; edge_id < route_info->edge_count; ++edge_id) {
            Graph::Edge<double> edge = graph_->GetEdge(router_->GetRouteEdge(route_info->id, edge_id));
            result.total_time += edge.weight;
            const Vertex& vertex_from = vertex_by_id_.at(edge.from), vertex_to = vertex_by_id_.at(edge.to);
            if (vertex_to.bus.has_value()) {
                if (std::holds_alternative<RouteResponse::RouteWaitInfo>(curr_action)) {
                    result.actions.emplace_back(RouteResponse::RouteWaitInfo({edge.weight * 2, vertex_from.stop_name}));
                    curr_action = RouteResponse::RouteBusInfo{0, vertex_to.bus.value(), 0.0};
                } else {
                    auto& curr_action_ref = std::get<RouteResponse::RouteBusInfo> (curr_action);
                    if (vertex_to.bus.value() != curr_action_ref.bus_number) throw std::runtime_error("invalid vertex_to.bus");
                    ++curr_action_ref.span_count;
                    curr_action_ref.time += edge.weight;
                }
            } else {
                result.actions.push_back(curr_action);
                curr_action = RouteResponse::RouteWaitInfo{0, ""};
            }
        }
        router_->ReleaseRoute(route_info->id);
        return result;
    }
}