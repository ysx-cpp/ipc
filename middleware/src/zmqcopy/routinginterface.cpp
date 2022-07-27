// zeromq messages service procotol implement

#include "routinginterface.h"
#include <algorithm>
#include <glog/logging.h>
#include "zmq/zmq_config.h"
#include "messages_protos.pb.h"

namespace ipc {
namespace messages {
namespace zmqcopy {

bool send_zmq(std::shared_ptr<zmq::socket_t>& socket, std::mutex& mutex,
              const std::string& buffer) {
    try {
        zmq::message_t message(buffer.c_str(), buffer.length());
        std::lock_guard<std::mutex> lock(mutex);
        return socket->send(buffer.c_str(), buffer.length());
    } catch (zmq::error_t& e) {
        LOG(ERROR) << (e.what());
        return false;
    }
}

bool recv_zmq(std::shared_ptr<zmq::socket_t>& socket, std::mutex& mutex, std::string& buffer) {
    try {
        zmq::message_t message;
        {
            std::lock_guard<std::mutex> lock(mutex);

            if (socket->recv(&message) == false) {
                return false;
            }
        }
        buffer = std::string(static_cast<char*>(message.data()), message.size());
        return true;
    } catch (zmq::error_t& e) {
        LOG(ERROR) << (e.what());
        return false;
    }
}

bool send_message(std::shared_ptr<zmq::socket_t>& socket, std::mutex& mutex, const ::google::protobuf::Message& message) {
    // assert((std::is_base_of<::google::protobuf::Message, T>::value));

    std::string buffer;
    if (message.SerializeToString(&buffer) == true) {
        return send_zmq(socket, mutex, buffer);
    } else {
        return false;
    }
}

bool recv_message(std::shared_ptr<zmq::socket_t>& socket, std::mutex& mutex, ::google::protobuf::Message& message) {
    // assert((std::is_base_of<::google::protobuf::Message, T>::value));

    std::string buffer;
    if (recv_zmq(socket, mutex, buffer) == false) {
        return false;
    }
    return message.ParseFromString(buffer);
}

std::string splice(const std::string& protocol, const std::string& ip, int port)
{
    std::string addr;
    addr.append(protocol).append(ip).append(":").append(std::to_string(port));
    return addr;
}

////////////////////////////////////////////////////////////////////////////////
//class PublisherImpl
PublisherImpl::PublisherImpl(zmq::context_t &zmq_ctx)
{
    m_current_port = ipc_TOPIC_START_PORT;

    m_pub_socket = std::make_shared<zmq::socket_t>(zmq_ctx, ZMQ_PUB);
    m_pub_socket->setsockopt(ZMQ_SNDHWM, ipc_ZMQ_SEND_QUEUE);
    m_pub_socket->setsockopt(ZMQ_LINGER, ipc_ZMQ_CLOSE_WAIT);

    std::string server_pub_addr = splice("tcp://", ipc_SERVER_REP_ADDR, m_current_port + 1);
    m_pub_socket->bind(server_pub_addr.c_str());
}

void PublisherImpl::PublisherOnline(const RoutingMessage &request)
{
    // notify
    LOG(INFO) << "id:" << request.node().client_id() << " topic:" << request.node().message_topic();

    // assign port
    std::string pub_addr;
    std::string server_pub_addr = splice("tcp://", ipc_SERVER_REP_ADDR, m_current_port++);

    // reponse publisher addr, only one node
    std::string topic = request.node().message_topic();

    // boardcast publisher online
    RoutingMessage pub_message(request);
    pub_message.mutable_node()->set_socket_addr(pub_addr);
    send_message(m_pub_socket, m_pub_mutex, pub_message);

    // sync
    std::lock_guard<std::mutex> lock(m_topic_mutex);

    // append topic list
    m_pub_list.push_back(std::make_pair(topic, pub_addr));
}

void PublisherImpl::PublisherOffline(const RoutingMessage &request)
{
    // notify
    LOG(INFO) << "publisher_offline id: " << request.node().client_id() << " topic: " << request.node().message_topic();

    // boardcast publisher offline
    send_message(m_pub_socket, m_pub_mutex, request);

    // sync
    std::lock_guard<std::mutex> lock(m_topic_mutex);

    // delete topic node
    std::string topic = request.node().message_topic();
    std::string socket_addr = request.node().socket_addr();
    std::string client_id = request.node().client_id();
    std::string client_ip = request.node().client_ip();

    auto iter = std::find(m_pub_list.begin(), m_pub_list.end(), std::make_pair(topic, socket_addr));
    if (iter != m_pub_list.end())
    {
        m_pub_list.erase(iter);
    }
}

////////////////////////////////////////////////////////////////////////////////
// subscriber
SubscriberImpl::SubscriberImpl(zmq::context_t& zmq_ctx)
{
    stop_ = false;
    m_pool_timeout = ipc_ZMQ_POOL_TIMEOUT;

    m_sub_socket = std::make_shared<zmq::socket_t>(zmq_ctx, ZMQ_SUB);
    m_sub_socket->setsockopt(ZMQ_SNDHWM, ipc_ZMQ_SEND_QUEUE);
    m_sub_socket->setsockopt(ZMQ_LINGER, ipc_ZMQ_CLOSE_WAIT);

    std::string server_sub_addr = splice("tcp://", ipc_SERVER_REP_ADDR, ipc_TOPIC_START_PORT);
    m_sub_socket->connect(server_sub_addr.c_str());

    m_sub_socket->setsockopt(ZMQ_SUBSCRIBE, "", 0); // subscribe all

    // subscribe boardcast
    m_listen_thread = std::thread(&SubscriberImpl::ListenThread, this);
}

SubscriberImpl::~SubscriberImpl()
{
    // quit boardcast listen thread
    stop_ = true;
    if (m_listen_thread.joinable())
    {
        m_listen_thread.join();
    }
}

bool SubscriberImpl::Connected() const
{
    return m_sub_socket != nullptr;
}

void SubscriberImpl::ListenThread()
{
    // ipc::util::set_thread_name("imr-" + m_client_id);

    RoutingMessage message;
    zmq::pollitem_t zmq_pool_item = {*m_sub_socket, 0, ZMQ_POLLIN, 0};
    while (!stop_)
    {
        try
        {
            int rc = zmq::poll(&zmq_pool_item, 1, m_pool_timeout);
            if (rc < 0)
            {
                continue;
            }

            if (zmq_pool_item.revents & ZMQ_POLLIN)
            {
                if (recv_message(m_sub_socket, m_sub_mutex, message) == false)
                {
                    continue;
                }

                std::string topic = message.node().message_topic();
                std::string addr = message.node().socket_addr();

                if (message.action() == ROUTING_PUB_ONLINE)
                {
                    // notify messages client
                    m_notify_online(topic, addr);
                }
                else if (message.action() == ROUTING_PUB_OFFLINE)
                {
                    // notify messages client
                    m_notify_offline(topic, addr);
                }
                else
                {
                    LOG(ERROR) << ("unknown boardcast message");
                }
            }
        }
        catch (zmq::error_t &e)
        {
            LOG(ERROR) << (e.what());
        }
    }
}

void SubscriberImpl::EventCallback(RoutingEventCallback publisher_online, RoutingEventCallback publisher_offline)
{
    m_notify_online = publisher_online;
    m_notify_offline = publisher_offline;
}

////////////////////////////////////////////////////////////////////////////////
// routing server
ServerImpl::ServerImpl(zmq::context_t &zmq_ctx) : pub_impl_(zmq_ctx)
{
    // init status
    stop_ = false;
    m_pool_timeout = ipc_ZMQ_POOL_TIMEOUT;
    m_current_port = ipc_TOPIC_START_PORT;
    m_node_list = std::make_shared<SubscribeNodeList>();

    // init zmq
    // static zmq::context_t zmq_ctx = zmq::context_t(1);

    m_rep_socket = std::make_shared<zmq::socket_t>(zmq_ctx, ZMQ_REP);
    m_rep_socket->setsockopt(ZMQ_SNDHWM, ipc_ZMQ_RECV_QUEUE);
    m_rep_socket->setsockopt(ZMQ_LINGER, ipc_ZMQ_CLOSE_WAIT);

    std::string server_rep_addr = splice("tcp://", ipc_SERVER_REP_ADDR, m_current_port);
    m_rep_socket->bind(server_rep_addr.c_str());
}

ServerImpl::~ServerImpl()
{
    this->Stop();
}

void ServerImpl::Run()
{
    RoutingMessage request;
    zmq::pollitem_t zmq_pool_item = {*m_rep_socket, 0, ZMQ_POLLIN, 0};
    while (!stop_)
    {
        try
        {
            int rc = zmq::poll(&zmq_pool_item, 1, m_pool_timeout);
            if (rc < 0)
                continue;

            if (zmq_pool_item.revents & ZMQ_POLLIN)
            {
                // do not use recv_message<T>() for get "Peer-Address"
                zmq::message_t message;
                // sync block
                {
                    std::lock_guard<std::mutex> lock(m_rep_mutex);
                    if (m_rep_socket->recv(&message, ZMQ_DONTWAIT) == false)
                    {
                        continue;
                    }
                }
                // deserialization
                std::string buffer = std::string(static_cast<char *>(message.data()), message.size());
                if (request.ParseFromString(buffer) == false)
                {
                    continue;
                }

                // client ip
                request.mutable_node()->set_client_ip(message.gets("Peer-Address"));

                switch (request.action())
                {
                case ROUTING_PUB_ONLINE:
                    request.mutable_node()->set_node_type(ROUTING_NODE_PUB);
                    pub_impl_.PublisherOnline(request);
                    break;
                case ROUTING_PUB_OFFLINE:
                    request.mutable_node()->set_node_type(ROUTING_NODE_PUB);
                    pub_impl_.PublisherOffline(request);
                    break;
                case ROUTING_SUB_ONLINE:
                    request.mutable_node()->set_node_type(ROUTING_NODE_SUB);
                    ResponseOnline(request);
                    break;
                case ROUTING_SUB_OFFLINE:
                    request.mutable_node()->set_node_type(ROUTING_NODE_SUB);
                    ResponseOffline(request);
                    break;
                case ROUTING_NODE_LIST:
                    ResponseNodeList();
                    break;
                default:
                    LOG(ERROR) << ("unknown protocol request");
                    break;
                }
            }
        }
        catch (zmq::error_t &e)
        {
            LOG(ERROR) << (e.what());
        }
    }
}

void ServerImpl::Stop()
{
    stop_ = true;
}

void ServerImpl::ResponseOnline(const RoutingMessage &request)
{
    // notify
    LOG(WARNING) << "subscriber_online id: " << request.node().client_id() << " topic: " << request.node().message_topic();

    // reponse publisher addr list
    RoutingMessageList response;
    response.set_action(request.action());

    std::string topic = request.node().message_topic();
    send_message(m_rep_socket, m_rep_mutex, response);

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

void ServerImpl::ResponseOffline(const RoutingMessage &request)
{
    // notify
    LOG(WARNING) << "subscriber_offline id: " << request.node().client_id() << " topic: " << request.node().message_topic();
    // response offline
    send_message(m_rep_socket, m_rep_mutex, request);

    std::string topic = request.node().message_topic();
    std::string client_id = request.node().client_id();
    std::string client_ip = request.node().client_ip();

    // sync
    std::lock_guard<std::mutex> lock(m_topic_mutex);

    // delete subscriber node
    auto node_list = m_node_list->mutable_node_list();
    for (auto iter = node_list->begin(); iter != node_list->end(); ++iter)
    {
        if ((iter->node_type() == ROUTING_NODE_SUB) && (iter->message_topic() == topic) &&
            (iter->client_id() == client_id) && (iter->client_ip() == client_ip))
        {
            node_list->erase(iter);
            break;
        }
    }
}

void ServerImpl::ResponseNodeList()
{
    // response offline
    send_message(m_rep_socket, m_rep_mutex, *m_node_list);
}

////////////////////////////////////////////////////////////////////////////////
// routing client
ReqClient::ReqClient(zmq::context_t& zmq_ctx) 
{
    /*
    // load config
    MessagesConfig config;
    bool parse_result = ipc::util::load_config<MessagesConfig>(ipc_CONFIG_MESSAGES, config);
    if (parse_result == false) {
        LOG(ERROR) << ("config [%s] lost", ipc_CONFIG_MESSAGES);
        return;
    }
    */

    // init zmq
    // zmq::context_t& zmq_ctx = ContextManager::instance();

    m_req_socket = std::make_shared<zmq::socket_t>(zmq_ctx, ZMQ_REQ);
    m_req_socket->setsockopt(ZMQ_SNDHWM, ipc_ZMQ_RECV_QUEUE);
    m_req_socket->setsockopt(ZMQ_LINGER, ipc_ZMQ_CLOSE_WAIT);

    std::string server_req_addr = splice("tcp://", ipc_SERVER_REP_ADDR, ipc_TOPIC_START_PORT);
    m_req_socket->connect(server_req_addr.c_str());
}

ReqClient::~ReqClient() 
{
    // offline all pub & sub addr
    for (auto& topic : m_advertised_topic) 
    {
        this->RequstNodeOffline(ROUTING_PUB_OFFLINE, topic.first, topic.second);
    }

    for (auto &topic : m_subscribed_topic)
    {
        this->RequstNodeOffline(ROUTING_SUB_OFFLINE, topic.first, topic.second);
    }
}

bool ReqClient::Connected() const 
{
    return m_req_socket != nullptr;
}

bool ReqClient::LookupPubAddr(const std::string &topic, std::string &pub_addr)
{
    assert(topic != "");

    std::vector<std::string> pub_addr_list;
    bool res = RequestNodeOnline(ROUTING_PUB_ONLINE, topic, pub_addr_list);
    if (res && (pub_addr_list.size() > 0))
    {
        // only one response
        m_advertised_topic[topic] = pub_addr_list[0];
        pub_addr = pub_addr_list[0];
    }
    return res;
}

bool ReqClient::LookupSubAddr(const std::string &topic, std::vector<std::string> &sub_addr_list)
{
    assert(topic != "");

    bool res = this->RequestNodeOnline(ROUTING_SUB_ONLINE, topic, sub_addr_list);
    if (res)
    {
        for (auto &addr : sub_addr_list)
        {
            m_subscribed_topic.push_back(std::make_pair(topic, addr));
        }
    }
    return res;
}

bool ReqClient:: RequestNodeOnline(const int32_t action, const std::string& topic,
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
    if (send_message(m_req_socket, m_req_mutex, message) == false) {
        return false;
    }

    // response
    RoutingMessageList response;
    if (recv_message(m_req_socket, m_req_mutex, response) == false) {
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

bool ReqClient::RequstNodeOffline(const int32_t action, const std::string& topic, const std::string& addr) {
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
    if (send_message(m_req_socket, m_req_mutex, message) == false) {
        return false;
    }

    // response
    if (recv_message(m_req_socket, m_req_mutex, message) == false) {
        return false;
    }

    return true;
}

bool ReqClient::RequestNodeList(SubscribeNodeList& node_list) {
    RoutingMessage message;
    message.set_action(ROUTING_NODE_LIST);
    message.mutable_node()->clear_message_topic();
    message.mutable_node()->set_client_id(m_client_id);
    message.mutable_node()->clear_client_ip();
    message.mutable_node()->clear_socket_addr();

    // request
    if (send_message(m_req_socket, m_req_mutex, message) == false) {
        return false;
    }

    // response
    if (recv_message(m_req_socket, m_req_mutex, node_list) == false) {
        return false;
    }

    return true;
}
} // namespace zmqcopy 
} // namespace messages
} // namespace ipc
