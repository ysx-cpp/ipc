// zeromq messages service procotol interface

#pragma once
#include <string>
#include <map>
#include <memory>
#include <atomic>
#include <mutex>
#include <thread>
#include <utility>
#include <functional>
#include <unordered_map>
#include "zmq/zmq.hpp"

// forward declaration for hiding zmq.hpp
namespace zmq {
    class socket_t;
}

// node type defines
#define ROUTING_NODE_PUB    1
#define ROUTING_NODE_SUB    2

// routing action defines
#define ROUTING_PUB_ONLINE  1
#define ROUTING_PUB_OFFLINE 2
#define ROUTING_SUB_ONLINE  3
#define ROUTING_SUB_OFFLINE 4
#define ROUTING_NODE_LIST   5

namespace ipc {
namespace messages {

// forward declaration for hiding messages_protos.pb.h
class RoutingMessage;
class SubscribeNodeList;

namespace zmqcopy {

////////////////////////////////////////////////////////////////////////////////
class PublisherImpl
{
public:
    explicit PublisherImpl(zmq::context_t& zmq_ctx);
    bool Publish(const RoutingMessage& message);

private:
    std::mutex pub_mutex_;
    std::unique_ptr<zmq::socket_t> pub_socket_;
};

////////////////////////////////////////////////////////////////////////////////
class SubscriberImpl
{
public:
    using SubscribeCallback = std::function<void(const RoutingMessage&)>;
    explicit SubscriberImpl(zmq::context_t& zmq_ctx);
    ~SubscriberImpl();

    bool Connected() const;
    void Run(SubscribeCallback&& callback);
    void Stop();

private:
    std::atomic<bool> stop_;
    int32_t pool_timeout_;

    std::mutex sub_mutex_;
    std::unique_ptr<zmq::socket_t> sub_socket_;
};

class ResponseImpl
{
    using RequestCallback = std::function<void(const RoutingMessage&)>;
public:
    explicit ResponseImpl(zmq::context_t& zmq_ctx);
    ~ResponseImpl();

    void Run(RequestCallback&& callback);
    void Stop();

private:
    std::atomic<bool> stop_;
    int32_t m_pool_timeout;

    std::mutex rep_mutex_;
    std::unique_ptr <zmq::socket_t> rep_socket_;
};

////////////////////////////////////////////////////////////////////////////////
class RequestImpl
{
public:
    explicit RequestImpl(zmq::context_t& zmq_ctx);

    bool Connected() const;
    bool Request(const RoutingMessage& request, RoutingMessage& response);

private:
    std::mutex req_mutex_;
    std::unique_ptr<zmq::socket_t> req_socket_;
};

////////////////////////////////////////////////////////////////////////////////
class RouterImpl
{
public:
    explicit RouterImpl(zmq::context_t& zmq_ctx);

    void Run();

private:
    std::atomic<bool> stop_;
    int32_t pool_timeout_;

private:
    std::mutex frontend_route_mutex_;
    std::unique_ptr<zmq::socket_t> frontend_router_socket_;

    std::mutex backend_route_mutex_;
    std::unique_ptr<zmq::socket_t> backend_router_socket_;
};

} // namespace zmqcopy 
} // namespace messages
} // namespace ipc
