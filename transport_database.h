#pragma once

#ifndef CPPCOURSERA_TRANSPORT_DIRECTORY_H
#define CPPCOURSERA_TRANSPORT_DIRECTORY_H

#endif //CPPCOURSERA_TRANSPORT_DIRECTORY_H

#include "point.h"
#include <iostream>
#include <memory>
#include <string>
#include <sstream>
#include <variant>
#include <vector>
#include <unordered_map>

#include "profile.h"
#include "transport.h"
#include "json.h"
#include "router.h"

namespace Transport {
    class TransportDatabase {
    public:
        void AddRoutingSettings(RouteSettings route_settings);
        void AddStop(StopHandler stop);
        void AddBus(BusHandler bus);
        Json::Node GetBus(const BusNumber& number, size_t request_id) const;
        Json::Node GetStop(const StopName& name, size_t request_id) const;
        Json::Node GetRoute(const StopName& from, const StopName& to, size_t request_id) const;
        void InitializeRouter();
    private:
        struct Vertex {
            StopName stop_name;
            std::optional<BusNumber> bus;
        };
        std::unique_ptr<Graph::Router<double>> router_;
        std::unique_ptr<Graph::DirectedWeightedGraph<double>> graph_;
        RouteSettings route_settings_;
        std::unordered_map<StopName, StopHandler> stop_by_name_;
        std::unordered_map<BusNumber, BusHandler> bus_by_number_;
        std::unordered_map<StopName, Graph::VertexId> abstract_id_by_name_;
        std::unordered_map<Graph::VertexId, Vertex> vertex_by_id_;
        static Json::Node NotFound(size_t request_id);
        void InitializeGraph();
        std::optional<RouteResponse> BuildRoute(const StopName& from, const StopName& to) const;
        template <typename RandomIt>
        void AddBusRouteToGraph(RandomIt begin, RandomIt end, size_t& vertex_count, const BusNumber& bus_number) {
            for (RandomIt it = begin; it != end; ++it) {
                Graph::VertexId abstract_stop = abstract_id_by_name_.at((*it)->name);
                Graph::VertexId curr_stop = vertex_count++;
                vertex_by_id_[curr_stop] = {(*it)->name, bus_number};
                graph_->AddEdge({abstract_stop, curr_stop, static_cast<double>(route_settings_.bus_wait_time) / 2});
                graph_->AddEdge({curr_stop, abstract_stop, static_cast<double>(route_settings_.bus_wait_time) / 2});
                if (it != begin) {
                    Graph::VertexId prev_stop = vertex_count - 2;
                    double forward_time = (*std::prev(it))->distance_to_stops.at((*it)->name) / route_settings_.bus_velocity;
                    graph_->AddEdge({prev_stop, curr_stop, forward_time});
                }
            }
        }
    };
}

