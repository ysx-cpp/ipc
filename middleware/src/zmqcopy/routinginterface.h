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

// messages_client.callback(topic, addr)
using RoutingEventCallback = std::function<void(const std::string&, const std::string&)>;

////////////////////////////////////////////////////////////////////////////////
class PublisherImpl
{
public:
    explicit PublisherImpl(zmq::context_t& zmq_ctx);
    void PublisherOnline(const RoutingMessage& request);
    void PublisherOffline(const RoutingMessage &request);

private:
    std::atomic<int32_t> m_current_port;

    std::mutex m_pub_mutex;
    std::shared_ptr<zmq::socket_t> m_pub_socket;

    // topic + pub addr
    std::mutex m_topic_mutex;
    std::vector<std::pair<std::string, std::string>> m_pub_list;
};

////////////////////////////////////////////////////////////////////////////////
class SubscriberImpl
{
public:
    explicit SubscriberImpl(zmq::context_t& zmq_ctx);
    ~SubscriberImpl();
    bool Connected() const;
    void ListenThread();
    void EventCallback(RoutingEventCallback publisher_online, RoutingEventCallback publisher_offline);
    bool LookupSubAddr(const std::string &topic, std::vector<std::string> &sub_addr_list);

private:
    RoutingEventCallback m_notify_online;
    RoutingEventCallback m_notify_offline;

    std::mutex m_sub_mutex;
    std::shared_ptr<zmq::socket_t> m_sub_socket;

    std::atomic<bool> stop_;
    int32_t m_pool_timeout;
    std::thread m_listen_thread;
};

////////////////////////////////////////////////////////////////////////////////
class ServerInterface 
{
public:
    virtual ~ServerInterface() = 0;
    virtual void Run() = 0;
    virtual void Stop() = 0;

    // virtual void PublisherOnline(const RoutingMessage& request) = 0;
    // virtual void PublisherOffline(const RoutingMessage& request) = 0;
    // virtual void SubscriberOnline(const RoutingMessage& request) = 0;
    // virtual void SubscriberOffline(const RoutingMessage& request) = 0;
    // virtual void NodeList() = 0;

protected:
    std::atomic<bool> stop_;
    std::atomic<int32_t> m_current_port;
    int32_t m_pool_timeout;
};
class ServerImpl : public ServerInterface
{
public:
    explicit ServerImpl(zmq::context_t& zmq_ctx);
    ~ServerImpl();
    void Run();
    void Stop();
    void ResponseOnline(const RoutingMessage &request);
    void ResponseOffline(const RoutingMessage &request);
    void ResponseNodeList();
private:
    std::atomic<bool> stop_;
    std::atomic<int32_t> m_current_port;
    int32_t m_pool_timeout;

private:
    // topic -> pub
    PublisherImpl pub_impl_;

    std::mutex m_rep_mutex;
    std::shared_ptr<zmq::socket_t> m_rep_socket;

    // topic -> sub list
    std::mutex m_topic_mutex;
    std::shared_ptr<SubscribeNodeList> m_node_list;
};

////////////////////////////////////////////////////////////////////////////////
class ReqClient
{
public:
    explicit ReqClient(zmq::context_t& zmq_ctx);
    virtual ~ReqClient();
    bool Connected() const;
    void Stop();
    bool LookupPubAddr(const std::string &topic, std::string &pub_addr);
    bool LookupSubAddr(const std::string &topic, std::vector<std::string> &sub_addr_list);
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
    std::shared_ptr<zmq::socket_t> m_req_socket;

    std::string m_client_id;
};

} // namespace messages
} // namespace ipc
