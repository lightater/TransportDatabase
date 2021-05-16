#include "requests.h"
#include "utils.h"

#include <algorithm>

#include <iostream>
#include <iomanip>

namespace Transport::Requests {
    Request::Request(Type type) : type(type) {}
    AddRoutingSettings::AddRoutingSettings(Type type, const Json::Node &body) : ModifyRequest(type) {
        route_settings.bus_wait_time = body.AsMap().at("bus_wait_time").AsInt();
        route_settings.bus_velocity = body.AsMap().at("bus_velocity").HoldsInt() ? body.AsMap().at("bus_velocity").AsInt() : body.AsMap().at("bus_velocity").AsDouble();
        route_settings.bus_velocity *= 50. / 3; // км/ч -> м/мин
    }
    void AddRoutingSettings::Process(TransportDatabase &tdb) const { tdb.AddRoutingSettings(route_settings); }
    AddStopRequest::AddStopRequest(Type type, const Json::Node& body) : ModifyRequest(type) {
        stop = std::make_shared<Stop>();
        stop->name = body.AsMap().at("name").AsString();
        const auto& latitude_node = body.AsMap().at("latitude"), longitude_node = body.AsMap().at("longitude");
        stop->location.latitude.value = latitude_node.HoldsInt() ? latitude_node.AsInt() : latitude_node.AsDouble();
        stop->location.longitude.value = longitude_node.HoldsInt() ? longitude_node.AsInt() : longitude_node.AsDouble();
        for (const auto& [stop_name, dist_node] : body.AsMap().at("road_distances").AsMap()) {
            stop->distance_to_stops[stop_name] = dist_node.AsInt();
        }
    }
    void AddStopRequest::Process(TransportDatabase &tdb) const { tdb.AddStop(stop); }
    AddBusRequest::AddBusRequest(Type type, const Json::Node& body) : ModifyRequest(type) {
        bus = std::make_shared<Bus>();
        bus->number = body.AsMap().at("name").AsString();
        BusRoute::Type bus_type = body.AsMap().at("is_roundtrip").AsBool() ? BusRoute::Type::Circular : BusRoute::Type::Direct;
        std::vector<StopName> stop_names;
        for (const auto& stop_node : body.AsMap().at("stops").AsArray()) {
            stop_names.push_back(stop_node.AsString());
        }
        bus->route = BusRoute {bus_type, stop_names};
    }
    void InitializeRouterRequest::Process(TransportDatabase &tdb) const { tdb.InitializeRouter(); }
    void AddBusRequest::Process(TransportDatabase &tdb) const { tdb.AddBus(bus); }
    GetBusRequest::GetBusRequest(Type type, const Json::Node& body) : ReadRequest(type, body) {
        bus_number = body.AsMap().at("name").AsString();
    }
    Json::Node GetBusRequest::Process(const TransportDatabase &tdb) const { return tdb.GetBus(bus_number, id); }
    GetStopRequest::GetStopRequest(Type type, const Json::Node& body) : ReadRequest(type, body) {
        stop_name = body.AsMap().at("name").AsString();
    }
    Json::Node GetStopRequest::Process(const TransportDatabase &tdb) const { return tdb.GetStop(stop_name, id); }
    GetRouteRequest::GetRouteRequest(Type type, const Json::Node &body) : ReadRequest(type, body) {
        from = body.AsMap().at("from").AsString();
        to = body.AsMap().at("to").AsString();
    }
    Json::Node GetRouteRequest::Process(const TransportDatabase &tdb) const { return tdb.GetRoute(from, to, id); }
    RequestHolder ParseRequest(Request::Type type, const Json::Node& request_body) {
        switch (type) {
            case Request::Type::AddRoutingSettings:
                return std::make_unique<AddRoutingSettings>(type, request_body);
            case Request::Type::AddStop:
                return std::make_unique<AddStopRequest>(type, request_body);
            case Request::Type::AddBus:
                return std::make_unique<AddBusRequest>(type, request_body);
            case Request::Type::GetBus:
                return std::make_unique<GetBusRequest>(type, request_body);
            case Request::Type::GetStop:
                return std::make_unique<GetStopRequest>(type, request_body);
            case Request::Type::GetRoute:
                return std::make_unique<GetRouteRequest>(type, request_body);
            default:
                throw std::runtime_error("unknown type parameter");
        }
    }
    std::vector<RequestHolder> ParseRequests(const Json::Document& document) {
        std::vector<RequestHolder> requests;
        const auto& root = document.GetRoot();
        if (root.AsMap().count("routing_settings") == 0) throw std::invalid_argument("Json document doesn't contains routing_settings");
        if (root.AsMap().count("base_requests") == 0) throw std::invalid_argument("Json document doesn't contains base_requests");
        if (root.AsMap().count("stat_requests") == 0) throw std::invalid_argument("Json document doesn't contains stat_requests");
        for (const auto& request_json : root.AsMap().at("base_requests").AsArray()) {
            auto request_json_map = request_json.AsMap();
            std::string type = request_json_map.at("type").AsString();
            if (type == "Stop") {
                requests.push_back(ParseRequest(Request::Type::AddStop, request_json));
            } else if (type == "Bus") {
                requests.push_back(ParseRequest(Request::Type::AddBus, request_json));
            } else {
                throw std::invalid_argument("unknown type of request");
            }
        }
        std::partition(requests.begin(), requests.end(), [](const RequestHolder& lhs){
            return lhs->type == Request::Type::AddStop;
        });
        requests.push_back(ParseRequest(Request::Type::AddRoutingSettings, root.AsMap().at("routing_settings").AsMap()));
        requests.push_back(std::make_unique<InitializeRouterRequest>(Request::Type::InitializeRouter));
        for (const auto& request_json : root.AsMap().at("stat_requests").AsArray()) {
            auto request_json_map = request_json.AsMap();
            std::string type = request_json_map.at("type").AsString();
            if (type == "Stop") {
                requests.push_back(ParseRequest(Request::Type::GetStop, request_json));
            } else if (type == "Bus") {
                requests.push_back(ParseRequest(Request::Type::GetBus, request_json));
            } else if (type == "Route") {
                requests.push_back(ParseRequest(Request::Type::GetRoute, request_json));
            } else {
                throw std::invalid_argument("unknown type of request");
            }
        }
        return requests;
    }
    Json::Document ProcessRequests(const std::vector<RequestHolder>& requests, TransportDatabase& tdb) {
        std::vector<Json::Node> request_results;
        for (const auto& request : requests) {
            if (request->type == Request::Type::AddStop || request->type == Request::Type::AddBus ||
                request->type == Request::Type::AddRoutingSettings || request->type == Request::Type::InitializeRouter) {
                const auto& req = dynamic_cast<const ModifyRequest&>(*request);
                req.Process(tdb);
            } else {
                const auto& req = dynamic_cast<const ReadRequest<Json::Node>&>(*request);
                request_results.push_back(req.Process(tdb));
            }
        }
        return Json::Document(Json::Node(request_results));
    }
    std::ostream& operator << (std::ostream& output, const Request::Type& type) {
        return output << static_cast<int>(type);
    }
}