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
    void PublisherOnline(const RoutingMessage& request);
    void PublisherOffline(const RoutingMessage& request);

private:
    std::atomic<int32_t> m_current_port;

    std::mutex m_pub_mutex;
    std::unique_ptr<zmq::socket_t> m_pub_socket;

    // topic + pub addr
    std::mutex m_topic_mutex;
    // std::unordered_map<std::string, std::string> pub_list_;
    std::unique_ptr<SubscribeNodeList> pub_list_;
};

////////////////////////////////////////////////////////////////////////////////
class SubscriberImpl
{
    using SubscriberCallback = std::function<void(const RoutingMessage&)>;
public:
    explicit SubscriberImpl(zmq::context_t& zmq_ctx);
    ~SubscriberImpl();
    bool Connected() const;
    void Run(SubscriberCallback&& callback);
    void Stop();

private:
    std::atomic<bool> stop_;
    int32_t m_pool_timeout;

    std::mutex m_sub_mutex;
    std::unique_ptr<zmq::socket_t> m_sub_socket;
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

    std::mutex m_rep_mutex;
    std::unique_ptr <zmq::socket_t> m_rep_socket;
};

////////////////////////////////////////////////////////////////////////////////
class RequestImpl
{
public:
    explicit RequestImpl(zmq::context_t& zmq_ctx);
    virtual ~RequestImpl();

    bool Connected() const;
    bool Request(const RoutingMessage& message);
    bool Response(RoutingMessage& response);

private:
    std::mutex m_req_mutex;
    std::unique_ptr<zmq::socket_t> m_req_socket;
};
} // namespace zmqcopy 
} // namespace messages
} // namespace ipc
