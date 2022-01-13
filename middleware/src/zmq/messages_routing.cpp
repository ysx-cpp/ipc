// zeromq messages service procotol implement

#include <algorithm>
#include <glog/logging.h>
#include "zmq.hpp"
#include "zmq_context.h"
#include "messages_routing.h"
#include "messages_protos.pb.h"
#include "utils/util_serializer.h"
#include "utils/util_string.h"

namespace ipc {
namespace util {
namespace messages {

////////////////////////////////////////////////////////////////////////////////
// routing server

RoutingServer::RoutingServer(const uint16_t port) {
    /*
    // load config
    MessagesConfig config;
    bool parse_result = ipc::util::load_config<MessagesConfig>(ipc_CONFIG_MESSAGES, config);
    if (parse_result == false) {
        DEBUG_ERROR("config [%s] lost", ipc_CONFIG_MESSAGES);
        return;
    }
    */

    // init status
    m_exit_signal = false;
    m_pool_timeout = ipc_ZMQ_POOL_TIMEOUT;
    m_current_port = ipc_TOPIC_START_PORT;
    m_node_list = std::make_shared<SubscribeNodeList>();

    // init zmq
    zmq::context_t& zmq_ctx = ContextManager::instance();

    m_rep_socket = std::make_shared<zmq::socket_t>(zmq_ctx, ZMQ_REP);
    m_rep_socket->setsockopt(ZMQ_SNDHWM, ipc_ZMQ_RECV_QUEUE);
    m_rep_socket->setsockopt(ZMQ_LINGER, ipc_ZMQ_CLOSE_WAIT);

    std::string server_rep_addr = ipc::util::string_sprintf("tcp://%s:%d", ipc_SERVER_REP_ADDR, port);
    m_rep_socket->bind(server_rep_addr.c_str());

    m_pub_socket = std::make_shared<zmq::socket_t>(zmq_ctx, ZMQ_PUB);
    m_pub_socket->setsockopt(ZMQ_SNDHWM, ipc_ZMQ_SEND_QUEUE);
    m_pub_socket->setsockopt(ZMQ_LINGER, ipc_ZMQ_CLOSE_WAIT);

    std::string server_pub_addr = ipc::util::string_sprintf("tcp://%s:%d", ipc_SERVER_PUB_ADDR, port + 1);
    m_pub_socket->bind(server_pub_addr.c_str());
}

RoutingServer::~RoutingServer() {
    this->stop();
}

void RoutingServer::stop() {
    m_exit_signal = true;
}

void RoutingServer::run() {
    RoutingMessage request;
    ipc::util::Serializer<RoutingMessage> serializer;
    zmq::pollitem_t zmq_pool_item = {*m_rep_socket, 0, ZMQ_POLLIN, 0};
    while (!m_exit_signal) {
        try {
            int rc = zmq::poll(&zmq_pool_item, 1, m_pool_timeout);
            if (rc < 0) {
                continue;
            }

            if (zmq_pool_item.revents & ZMQ_POLLIN) {

                // do not use recv_message<T>() for get "Peer-Address"
                zmq::message_t message;
                // sync block
                {
                    std::lock_guard<std::mutex> lock(m_rep_mutex);
                    if (m_rep_socket->recv(&message, ZMQ_DONTWAIT) == false) {
                        continue;
                    }
                }
                // deserialization
                std::string buffer = std::string(static_cast<char*>(message.data()), message.size());
                if (serializer.from_string(buffer, request) == false) {
                    continue;
                }

                // client ip
                request.mutable_node()->set_client_ip(message.gets("Peer-Address"));

                if (request.action() == ROUTING_PUB_ONLINE) {
                    request.mutable_node()->set_node_type(ROUTING_NODE_PUB);
                    this->publisher_online(request);
                } else if (request.action() == ROUTING_PUB_OFFLINE) {
                    request.mutable_node()->set_node_type(ROUTING_NODE_PUB);
                    this->publisher_offline(request);
                } else if (request.action() == ROUTING_SUB_ONLINE) {
                    request.mutable_node()->set_node_type(ROUTING_NODE_SUB);
                    this->subscriber_online(request);
                } else if (request.action() == ROUTING_SUB_OFFLINE) {
                    request.mutable_node()->set_node_type(ROUTING_NODE_SUB);
                    this->subscriber_offline(request);
                } else if (request.action() == ROUTING_NODE_LIST) {
                    this->node_list();
                } else {
                    DEBUG_ERROR("unknown protocol request");
                }
            }
        } catch (zmq::error_t& e) {
            DEBUG_ERROR(e.what());
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
void RoutingServer::publisher_online(const RoutingMessage& request) {
    // notify
    DEBUG_INFO("id [%s], topic[%s]", request.node().client_id().c_str(),
        request.node().message_topic().c_str());
    LOG(WARNING) << "publisher_online id: " << request.node().client_id() << " topic: " << request.node().message_topic();

    // assign port
    int32_t next_port = m_current_port++;
    std::string pub_addr = ipc::util::string_sprintf("tcp://%s:%d",
        request.node().client_ip().c_str(), next_port);

    // reponse publisher addr, only one node
    std::string topic = request.node().message_topic();

    RoutingMessageList response;
    response.set_action(request.action());
    auto node = response.add_node_list();
    node->set_node_type(ROUTING_NODE_PUB);
    node->set_message_topic(topic);
    node->set_socket_addr(pub_addr);
    node->set_client_id(request.node().client_id());
    node->set_client_ip(request.node().client_ip());

    send_message<RoutingMessageList>(m_rep_socket, m_rep_mutex, response);

    // boardcast publisher online
    RoutingMessage boardcast_message(request);
    boardcast_message.mutable_node()->set_socket_addr(pub_addr);
    send_message<RoutingMessage>(m_pub_socket, m_pub_mutex, boardcast_message);

    // sync
    std::lock_guard<std::mutex> lock(m_topic_mutex);

    // append topic list
    m_pub_list.push_back(std::make_pair(topic, pub_addr));

    // append subscriber node list
    auto pub_node = m_node_list->add_node_list();
    pub_node->set_node_type(ROUTING_NODE_PUB);
    pub_node->set_message_topic(topic);
    pub_node->set_socket_addr(pub_addr);
    pub_node->set_client_id(request.node().client_id());
    pub_node->set_client_ip(request.node().client_ip());
}

void RoutingServer::publisher_offline(const RoutingMessage& request) {
    // notify
    DEBUG_INFO("id [%s], topic[%s]", request.node().client_id().c_str(),
        request.node().message_topic().c_str());
    LOG(WARNING) << "publisher_offline id: " << request.node().client_id() << " topic: " << request.node().message_topic();
    // response offline
    send_message<RoutingMessage>(m_rep_socket, m_rep_mutex, request);

    // boardcast publisher offline
    send_message<RoutingMessage>(m_pub_socket, m_pub_mutex, request);

    // sync
    std::lock_guard<std::mutex> lock(m_topic_mutex);

    // delete topic node
    std::string topic = request.node().message_topic();
    std::string socket_addr = request.node().socket_addr();
    std::string client_id = request.node().client_id();
    std::string client_ip = request.node().client_ip();

    auto iter = std::find(m_pub_list.begin(), m_pub_list.end(), std::make_pair(topic, socket_addr));
    if (iter != m_pub_list.end()) {
        m_pub_list.erase(iter);
    }

    // delete subscriber node
    auto node_list = m_node_list->mutable_node_list();
    for (auto iter = node_list->begin(); iter != node_list->end(); ++iter) {
        if ((iter->node_type() == ROUTING_NODE_PUB) && (iter->message_topic() == topic) &&
            (iter->socket_addr() == socket_addr) && (iter->client_id() == client_id) &&
            (iter->client_ip() == client_ip)) {
            node_list->erase(iter);
            break;
        }
    }
}

void RoutingServer::subscriber_online(const RoutingMessage& request) {
    // notify
    DEBUG_INFO("id [%s], topic[%s]", request.node().client_id().c_str(),
        request.node().message_topic().c_str());
    LOG(WARNING) << "subscriber_online id: " << request.node().client_id() << " topic: " << request.node().message_topic();

    // reponse publisher addr list
    RoutingMessageList response;
    response.set_action(request.action());

    std::string topic = request.node().message_topic();
    for (auto& pub_node : m_pub_list) {
        if (pub_node.first == topic) {
            auto node = response.add_node_list();
            node->set_node_type(ROUTING_NODE_SUB);
            node->set_message_topic(pub_node.first);
            node->set_socket_addr(pub_node.second);
            node->clear_client_id();
            node->clear_client_ip();
        }
    }

    send_message<RoutingMessageList>(m_rep_socket, m_rep_mutex, response);

    // sync
    std::lock_guard<std::mutex> lock(m_topic_mutex);

    // append subscriber node list
    auto sub_node = m_node_list->add_node_list();
    sub_node->set_node_type(ROUTING_NODE_SUB);
    sub_node->set_message_topic(topic);
    sub_node->set_socket_addr(request.node().socket_addr());
    sub_node->set_client_id(request.node().client_id());
    sub_node->set_client_ip(request.node().client_ip());
}

void RoutingServer::subscriber_offline(const RoutingMessage& request) {
    // notify
    DEBUG_INFO("id [%s], topic[%s]", request.node().client_id().c_str(),
        request.node().message_topic().c_str());
    LOG(WARNING) << "subscriber_offline id: " << request.node().client_id() << " topic: " << request.node().message_topic();
    // response offline
    send_message<RoutingMessage>(m_rep_socket, m_rep_mutex, request);

    std::string topic = request.node().message_topic();
    std::string client_id = request.node().client_id();
    std::string client_ip = request.node().client_ip();

    // sync
    std::lock_guard<std::mutex> lock(m_topic_mutex);

    // delete subscriber node
    auto node_list = m_node_list->mutable_node_list();
    for (auto iter = node_list->begin(); iter != node_list->end(); ++iter) {
        if ((iter->node_type() == ROUTING_NODE_SUB) && (iter->message_topic() == topic) &&
            (iter->client_id() == client_id) && (iter->client_ip() == client_ip)) {
            node_list->erase(iter);
            break;
        }
    }
}

void RoutingServer::node_list() {
    // response offline
    send_message<SubscribeNodeList>(m_rep_socket, m_rep_mutex, *m_node_list);
}

////////////////////////////////////////////////////////////////////////////////
// routing client

RoutingClient::RoutingClient(const std::string& client_id, const std::string server_addr, const uint16_t server_port) {
    assert(client_id != "");

    /*
    // load config
    MessagesConfig config;
    bool parse_result = ipc::util::load_config<MessagesConfig>(ipc_CONFIG_MESSAGES, config);
    if (parse_result == false) {
        DEBUG_ERROR("config [%s] lost", ipc_CONFIG_MESSAGES);
        return;
    }
    */

    // init status
    m_exit_signal = false;
    m_pool_timeout = ipc_ZMQ_POOL_TIMEOUT;
    m_client_id = client_id;

    // init zmq
    zmq::context_t& zmq_ctx = ContextManager::instance();

    m_req_socket = std::make_shared<zmq::socket_t>(zmq_ctx, ZMQ_REQ);
    m_req_socket->setsockopt(ZMQ_SNDHWM, ipc_ZMQ_RECV_QUEUE);
    m_req_socket->setsockopt(ZMQ_LINGER, ipc_ZMQ_CLOSE_WAIT);

    std::string server_req_addr = ipc::util::string_sprintf("tcp://%s:%d", server_addr.c_str(), server_port);
    m_req_socket->connect(server_req_addr.c_str());

    m_sub_socket = std::make_shared<zmq::socket_t>(zmq_ctx, ZMQ_SUB);
    m_sub_socket->setsockopt(ZMQ_SNDHWM, ipc_ZMQ_SEND_QUEUE);
    m_sub_socket->setsockopt(ZMQ_LINGER, ipc_ZMQ_CLOSE_WAIT);

    std::string server_sub_addr = ipc::util::string_sprintf("tcp://%s:%d", server_addr.c_str(), server_port + 1);
    m_sub_socket->connect(server_sub_addr.c_str());

    m_sub_socket->setsockopt(ZMQ_SUBSCRIBE, "", 0); // subscribe all

    // subscribe boardcast
    m_listen_thread = std::thread(&RoutingClient::listen_thread, this);
}

RoutingClient::~RoutingClient() {
    // offline all pub & sub addr
    for (auto& topic : m_advertised_topic) {
        this->node_offline(ROUTING_PUB_OFFLINE, topic.first, topic.second);
    }
    for (auto& topic : m_subscribed_topic) {
        this->node_offline(ROUTING_SUB_OFFLINE, topic.first, topic.second);
    }

    // quit boardcast listen thread
    m_exit_signal = true;
    if (m_listen_thread.joinable()) {
        m_listen_thread.join();
    }
}

bool RoutingClient::connected() {
    if (m_req_socket != nullptr && m_sub_socket != nullptr) {
        return true;
    } else {
        return false;
    }
}

void RoutingClient::stop() {
    m_exit_signal = true;
}

void RoutingClient::event_callback(RoutingEventCallback publisher_online, RoutingEventCallback publisher_offline) {
    m_notify_online = publisher_online;
    m_notify_offline = publisher_offline;
}

bool RoutingClient::lookup_pub_addr(const std::string& topic, std::string& pub_addr) {
    assert(topic != "");

    std::vector<std::string> pub_addr_list;
    bool res = this->node_online(ROUTING_PUB_ONLINE, topic, pub_addr_list);
    if (res && (pub_addr_list.size() > 0)) {
        // only one response
        m_advertised_topic[topic] = pub_addr_list[0];
        pub_addr = pub_addr_list[0];
    }
    return res;
}

bool RoutingClient::lookup_sub_addr(const std::string& topic, std::vector<std::string>& sub_addr_list) {
    assert(topic != "");

    bool res = this->node_online(ROUTING_SUB_ONLINE, topic, sub_addr_list);
    if (res) {
        for (auto& addr : sub_addr_list) {
            m_subscribed_topic.push_back(std::make_pair(topic, addr));
        }
    }
    return res;
}

////////////////////////////////////////////////////////////////////////////////
void RoutingClient::listen_thread() {
    ipc::util::set_thread_name("imr-" + m_client_id);

    RoutingMessage message;
    zmq::pollitem_t zmq_pool_item = {*m_sub_socket, 0, ZMQ_POLLIN, 0};
    while (!m_exit_signal) {
        try {
            int rc = zmq::poll(&zmq_pool_item, 1, m_pool_timeout);
            if (rc < 0) {
                continue;
            }

            if (zmq_pool_item.revents & ZMQ_POLLIN) {
                if (recv_message<RoutingMessage>(m_sub_socket, m_sub_mutex, message) == false) {
                    continue;
                }

                std::string topic = message.node().message_topic();
                std::string addr = message.node().socket_addr();

                if (message.action() == ROUTING_PUB_ONLINE) {
                    // notify messages client
                    m_notify_online(topic, addr);
                } else if (message.action() == ROUTING_PUB_OFFLINE) {
                    // notify messages client
                    m_notify_offline(topic, addr);
                } else {
                    DEBUG_ERROR("unknown boardcast message");
                }
            }
        } catch (zmq::error_t& e) {
            DEBUG_ERROR(e.what());
        }
    }
}

bool RoutingClient:: node_online(const int32_t action, const std::string& topic,
    std::vector<std::string>& addr_list) {
    assert(action > 0);
    assert(topic != "");

    RoutingMessage message;
    message.set_action(action);
    message.mutable_node()->clear_node_type();
    message.mutable_node()->set_message_topic(topic);
    message.mutable_node()->set_client_id(m_client_id);
    message.mutable_node()->clear_client_ip();
    message.mutable_node()->clear_socket_addr();

    // request
    if (send_message<RoutingMessage>(m_req_socket, m_req_mutex, message) == false) {
        return false;
    }

    // response
    RoutingMessageList response;
    if (recv_message<RoutingMessageList>(m_req_socket, m_req_mutex, response) == false) {
        return false;
    }

    if (response.node_list_size() > 0) {
        for (auto& node : response.node_list()) {
            addr_list.push_back(node.socket_addr()) ;
        }
        return true;
    } else {
        return false;
    }
}

bool RoutingClient::node_offline(const int32_t action, const std::string& topic, const std::string& addr) {
    assert(action > 0);
    assert(topic != "");

    RoutingMessage message;
    message.set_action(action);
    message.mutable_node()->clear_node_type();
    message.mutable_node()->set_message_topic(topic);
    message.mutable_node()->set_client_id(m_client_id);
    message.mutable_node()->clear_client_ip();
    message.mutable_node()->set_socket_addr(addr);

    // request
    if (send_message<RoutingMessage>(m_req_socket, m_req_mutex, message) == false) {
        return false;
    }

    // response
    if (recv_message<RoutingMessage>(m_req_socket, m_req_mutex, message) == false) {
        return false;
    }

    return true;
}

bool RoutingClient::node_list(SubscribeNodeList& node_list) {
    RoutingMessage message;
    message.set_action(ROUTING_NODE_LIST);
    message.mutable_node()->clear_message_topic();
    message.mutable_node()->set_client_id(m_client_id);
    message.mutable_node()->clear_client_ip();
    message.mutable_node()->clear_socket_addr();

    // request
    if (send_message<RoutingMessage>(m_req_socket, m_req_mutex, message) == false) {
        return false;
    }

    // response
    if (recv_message<SubscribeNodeList>(m_req_socket, m_req_mutex, node_list) == false) {
        return false;
    }

    return true;
}

} // namespace messages
} // namespace util
} // namespace ipc
