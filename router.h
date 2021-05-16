#pragma once

#include "graph.h"

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <iterator>
#include <optional>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>
#include <queue>

namespace Graph {

  template <typename Weight>
  class Router {
  private:
    using Graph = DirectedWeightedGraph<Weight>;

  public:
      Router(const Graph& graph, const std::vector<VertexId>& vertexes_to_compute);

    using RouteId = uint64_t;

    struct RouteInfo {
      RouteId id;
      Weight weight;
      size_t edge_count;
    };

    std::optional<RouteInfo> BuildRoute(VertexId from, VertexId to) const;
    EdgeId GetRouteEdge(RouteId route_id, size_t edge_idx) const;
    void ReleaseRoute(RouteId route_id);

  private:
    const Graph& graph_;

    struct RouteInternalData {
      Weight weight;
      std::optional<EdgeId> prev_edge;
    };
    using RoutesInternalDataMap = std::unordered_map<VertexId, std::unordered_map<VertexId, std::optional<RouteInternalData>>>;

    using ExpandedRoute = std::vector<EdgeId>;
    mutable RouteId next_route_id_ = 0;
    mutable std::unordered_map<RouteId, ExpandedRoute> expanded_routes_cache_;

      void InitializeRoutesInternalDataMap(const Graph& graph) {
          const size_t vertex_count = graph.GetVertexCount();
          for (VertexId vertex = 0; vertex < vertex_count; ++vertex) {
              routes_internal_data_map_[vertex][vertex] = RouteInternalData{0, std::nullopt};
              for (const EdgeId edge_id : graph.GetIncidentEdges(vertex)) {
                  const auto& edge = graph.GetEdge(edge_id);
                  assert(edge.weight >= 0);
                  if (routes_internal_data_map_[vertex].count(edge.to) == 0 ||
                  !routes_internal_data_map_[vertex][edge.to] || routes_internal_data_map_[vertex][edge.to]->weight > edge.weight) {
                      routes_internal_data_map_[vertex][edge.to] = RouteInternalData{edge.weight, edge_id};
                  }
              }
          }
      }

      void RelaxRouteMap(VertexId vertex_from, VertexId vertex_to,
                      const RouteInternalData& route_from, const RouteInternalData& route_to) {
          auto& route_relaxing = routes_internal_data_map_[vertex_from][vertex_to];
          const Weight candidate_weight = route_from.weight + route_to.weight;
          if (!route_relaxing || candidate_weight < route_relaxing->weight) {
              route_relaxing = {
                      candidate_weight,
                      route_to.prev_edge
                      ? route_to.prev_edge
                      : route_from.prev_edge
              };
          }
      }

      struct WeightVertexId {
          Weight weight;
          VertexId vertex_id;
          bool operator < (const WeightVertexId& other) const { return std::tie(weight, vertex_id) < std::tie(other.weight, other.vertex_id); }
      };
    void DijkstraAlgorithm(size_t vertex_count, VertexId vertex_from) {
        std::set<WeightVertexId> unused;
        unused.insert({0., vertex_from});
        std::unordered_set<VertexId> used;
        while (!unused.empty()) {
            WeightVertexId curr_wvi = *(unused.begin());
            unused.erase(unused.begin());
            used.insert(curr_wvi.vertex_id);
            for (const EdgeId edge_id : graph_.GetIncidentEdges(curr_wvi.vertex_id)) {
                auto edge = graph_.GetEdge(edge_id);
                if (routes_internal_data_map_[curr_wvi.vertex_id].count(edge.to) != 0;
                        const auto& second_route = routes_internal_data_map_[curr_wvi.vertex_id][edge.to]) {
                    RelaxRouteMap(vertex_from, edge.to, routes_internal_data_map_[vertex_from][curr_wvi.vertex_id].value(), second_route.value());
                    unused.insert({routes_internal_data_map_[vertex_from][edge.to]->weight, edge.to});
                }
            }
            while (!unused.empty() && used.count(unused.begin()->vertex_id) != 0) unused.erase(unused.begin());
        }
    }

    RoutesInternalDataMap routes_internal_data_map_;
  };

    template <typename Weight>
    Router<Weight>::Router(const Graph& graph, const std::vector<VertexId>& vertexes_to_compute)
            : graph_(graph)
    {
        InitializeRoutesInternalDataMap(graph);
        const size_t vertex_count = graph.GetVertexCount();
        for (auto vertex : vertexes_to_compute) DijkstraAlgorithm(vertex_count, vertex);
    }


  template <typename Weight>
  std::optional<typename Router<Weight>::RouteInfo> Router<Weight>::BuildRoute(VertexId from, VertexId to) const {
      if (routes_internal_data_map_.at(from).count(to) == 0) return std::nullopt;
    const auto& route_internal_data = routes_internal_data_map_.at(from).at(to);
    if (!route_internal_data) {
      return std::nullopt;
    }
    const Weight weight = route_internal_data->weight;
    std::vector<EdgeId> edges;
    for (std::optional<EdgeId> edge_id = route_internal_data->prev_edge;
         edge_id;
         edge_id = routes_internal_data_map_.at(from).at(graph_.GetEdge(*edge_id).from)->prev_edge) {
      edges.push_back(*edge_id);
    }
    std::reverse(std::begin(edges), std::end(edges));

    const RouteId route_id = next_route_id_++;
    const size_t route_edge_count = edges.size();
    expanded_routes_cache_[route_id] = std::move(edges);
    return RouteInfo{route_id, weight, route_edge_count};
  }

  template <typename Weight>
  EdgeId Router<Weight>::GetRouteEdge(RouteId route_id, size_t edge_idx) const {
    return expanded_routes_cache_.at(route_id)[edge_idx];
  }

  template <typename Weight>
  void Router<Weight>::ReleaseRoute(RouteId route_id) {
    expanded_routes_cache_.erase(route_id);
  }
}
