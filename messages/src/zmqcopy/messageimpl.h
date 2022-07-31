#pragma once
#include <string>
#include <map>
#include <memory>
#include <atomic>
#include <mutex>
#include <thread>
#include <utility>
#include <functional>
#include "zmq.hpp"
#include "streambuffer.hpp"

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

////////////////////////////////////////////////////////////////////////////////
class PublisherImpl
{
public:
    explicit PublisherImpl(zmq::context_t& zmq_ctx);
    void Bind(const std::string& host);
    bool Publish(const RoutingMessage& message);

private:
    StreamBuffer buffer_;
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
    std::chrono::milliseconds pool_timeout_;

    StreamBuffer buffer_;
    std::unique_ptr<zmq::socket_t> sub_socket_;
};

class ReplyImpl
{
    using RequestCallback = std::function<void(const RoutingMessage&)>;
public:
    explicit ReplyImpl(zmq::context_t& zmq_ctx);
    ~ReplyImpl();

    void Bind(const std::string& host);
    void Run(RequestCallback&& callback);
    bool SendResponse(const RoutingMessage& message);
    void Stop();

private:
    std::atomic<bool> stop_;
    std::chrono::milliseconds pool_timeout_;

    StreamBuffer buffer_;
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
    StreamBuffer send_buffer_;
    StreamBuffer recv_buffer_;
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
    std::chrono::milliseconds pool_timeout_;

private:
    StreamBuffer frontend_buffer_;
    std::unique_ptr<zmq::socket_t> frontend_router_socket_;

    StreamBuffer backend_buffer_;
    std::unique_ptr<zmq::socket_t> backend_router_socket_;
};

} // namespace messages
} // namespace ipc
