#pragma once

#ifndef CPPCOURSERA_REQUESTS_H
#define CPPCOURSERA_REQUESTS_H

#endif //CPPCOURSERA_REQUESTS_H

#include "transport_database.h"
#include "json.h"

namespace Transport::Requests {
    using RequestId = size_t;
    struct Request {
        enum class Type {
            AddRoutingSettings,
            AddStop,
            AddBus,
            InitializeRouter,
            GetBus,
            GetStop,
            GetRoute
        } type;
        explicit Request(Type type);
        virtual ~Request() = default;
    };
    template <typename ResultType>
    struct ReadRequest : Request {
        using Request::Request;
        ReadRequest(Type type, const Json::Node& body) : Request(type), id(body.AsMap().at("id").AsInt()) {}
        virtual ResultType Process(const TransportDatabase& tdb) const = 0;
        const RequestId id{0};
    };
    struct ModifyRequest : Request {
        using Request::Request;
        virtual void Process(TransportDatabase& tdb) const = 0;
    };
    struct AddRoutingSettings : ModifyRequest {
        AddRoutingSettings(Type type, const Json::Node& body);
        void Process(TransportDatabase& tdb) const override;
        RouteSettings route_settings;
    };
    struct AddStopRequest : ModifyRequest {
        AddStopRequest(Type type, const Json::Node& body);
        void Process(TransportDatabase& tdb) const override;
        StopHandler stop;
    };
    struct AddBusRequest : ModifyRequest {
        AddBusRequest(Type type, const Json::Node& body);
        void Process(TransportDatabase& tdb) const override;
        BusHandler bus;
    };
    struct InitializeRouterRequest : ModifyRequest {
        using ModifyRequest::ModifyRequest;
        void Process(TransportDatabase& tdb) const override;
    };
    struct GetBusRequest : ReadRequest<Json::Node> {
        GetBusRequest(Type type, const Json::Node& body);
        Json::Node Process(const TransportDatabase& tdb) const override;
        BusNumber bus_number;
    };
    struct GetStopRequest : ReadRequest<Json::Node> {
        GetStopRequest(Type type, const Json::Node& body);
        Json::Node Process(const TransportDatabase& tdb) const override;
        StopName stop_name;
    };
    struct GetRouteRequest : ReadRequest<Json::Node> {
        GetRouteRequest(Type type, const Json::Node& body);
        Json::Node Process(const TransportDatabase& tdb) const override;
        StopName from, to;
    };
    using RequestHolder = std::unique_ptr<Request>;
    RequestHolder ParseRequest(Request::Type type, const Json::Node& request_body);
    std::vector<RequestHolder> ParseRequests(const Json::Document&);
    Json::Document ProcessRequests(const std::vector<RequestHolder>&, TransportDatabase&);
    std::ostream& operator << (std::ostream&, const Request::Type&);
}