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
    // EventCallback(topic, addr)
    using EventCallback = std::function<void(const RoutingMessage&)>;
public:
    explicit SubscriberImpl(zmq::context_t& zmq_ctx);
    ~SubscriberImpl();
    bool Connected() const;
    void Stop();
    void Run(EventCallback&& callback);

private:
    std::atomic<bool> stop_;
    int32_t m_pool_timeout;

    std::mutex m_sub_mutex;
    std::unique_ptr<zmq::socket_t> m_sub_socket;
};

class ResponseImpl
{
public:
    explicit ResponseImpl(zmq::context_t& zmq_ctx);
    ~ResponseImpl();
    void Run();
    void Stop();
    void OnRequest(const RoutingMessage& request);
    void OnRequestSubOnline(const RoutingMessage& request);
    void OnRequestSubOffline(const RoutingMessage& request);
    void OnRequestNodeList();
private:
    std::atomic<bool> stop_;
    std::atomic<int32_t> m_current_port;
    int32_t m_pool_timeout;

private:
    std::mutex m_rep_mutex;
    std::unique_ptr <zmq::socket_t> m_rep_socket;

    // topic -> sub list
    std::mutex m_topic_mutex;
    std::unique_ptr<SubscribeNodeList> sub_list_;
};

////////////////////////////////////////////////////////////////////////////////
class RequestImpl
{
public:
    explicit RequestImpl(zmq::context_t& zmq_ctx);
    virtual ~RequestImpl();
    bool Connected() const;
    void Stop();
    bool GetPubAddr(const std::string &topic, std::string &pub_addr);
    bool GetSubAddr(const std::string &topic, std::vector<std::string> &sub_addr_list);
    bool RequestNodeList(SubscribeNodeList &node_list);

private:
    bool RequestNodeOnline(const int32_t action, const std::string &topic, std::vector<std::string> &addr_list);
    bool RequstNodeOffline(const int32_t action, const std::string &topic, const std::string &addr);

private:
    // topic -> pub socket addr
    std::map<std::string, std::string> m_advertised_topic;

    // topic + sub socket addr
    std::vector<std::pair<std::string, std::string>> m_subscribed_topic;

    std::mutex m_req_mutex;
    std::unique_ptr<zmq::socket_t> m_req_socket;

    std::string m_client_id;
};
} // namespace zmqcopy 
} // namespace messages
} // namespace ipc
