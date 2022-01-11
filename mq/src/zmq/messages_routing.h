// zeromq messages service procotol interface

#pragma once
#include <string>
#include <map>
#include <memory>
#include <atomic>
#include <mutex>
#include <utility>
#include <functional>
//#include "common/ipc_common.h"
#include "zmq_config.h"
#include "zmq_envelope.h"

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
namespace util {
namespace messages {

// forward declaration for hiding messages_protos.pb.h
class RoutingMessage;
class SubscribeNodeList;

// messages_client.callback(topic, addr)
using RoutingEventCallback = std::function<void(const std::string&, const std::string&)>;

////////////////////////////////////////////////////////////////////////////////
class RoutingServer {

public:
    RoutingServer(const uint16_t port);
    ~RoutingServer();
    void run();
    void stop();

private:
    void publisher_online(const RoutingMessage& request);
    void publisher_offline(const RoutingMessage& request);
    void subscriber_online(const RoutingMessage& request);
    void subscriber_offline(const RoutingMessage& request);
    void node_list();

private:
    std::mutex m_rep_mutex;
    std::mutex m_pub_mutex;
    std::mutex m_topic_mutex;
    std::shared_ptr<zmq::socket_t> m_rep_socket;
    std::shared_ptr<zmq::socket_t> m_pub_socket;

    int32_t m_pool_timeout;
    std::atomic<bool> m_exit_signal;
    std::atomic<int32_t> m_current_port;

    // topic + pub addr
    std::vector<std::pair<std::string, std::string>> m_pub_list;
    // topic -> sub list
    std::shared_ptr<SubscribeNodeList> m_node_list;
};

////////////////////////////////////////////////////////////////////////////////
class RoutingClient {

public:
    RoutingClient(const std::string& client_id, const std::string server_addr, const uint16_t server_port);
    ~RoutingClient();

    bool connected();
    void stop();
    void event_callback(RoutingEventCallback publisher_online,
        RoutingEventCallback publisher_offline);
    bool lookup_pub_addr(const std::string& topic, std::string& pub_addr);
    bool lookup_sub_addr(const std::string& topic, std::vector<std::string>& sub_addr_list);
    bool node_list(SubscribeNodeList& node_list);

private:
    void listen_thread();
    bool node_online(const int32_t action, const std::string& topic,
        std::vector<std::string>& addr_list);
    bool node_offline(const int32_t action, const std::string& topic,
        const std::string& addr);

private:
    RoutingEventCallback m_notify_online;
    RoutingEventCallback m_notify_offline;

    // topic -> pub socket addr
    std::map<std::string, std::string> m_advertised_topic;
    // topic + sub socket addr
    std::vector<std::pair<std::string, std::string>> m_subscribed_topic;

    std::mutex m_req_mutex;
    std::mutex m_sub_mutex;
    std::shared_ptr<zmq::socket_t> m_req_socket;
    std::shared_ptr<zmq::socket_t> m_sub_socket;

    int32_t m_pool_timeout;
    std::atomic<bool> m_exit_signal;
    std::string m_client_id;
    std::thread m_listen_thread;

};

} // namespace messages
} // namespace util
} // namespace ipc
