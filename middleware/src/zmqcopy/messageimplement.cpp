// zeromq messages service procotol implement

#include "messageimplement.h"
#include <algorithm>
#include <glog/logging.h>
#include "zmq/zmq_config.h"
#include "messages_protos.pb.h"

namespace ipc {
namespace messages {
namespace zmqcopy {

bool send_zmq(std::unique_ptr<zmq::socket_t>& socket, std::mutex& mutex,
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

bool recv_zmq(std::unique_ptr<zmq::socket_t>& socket, std::mutex& mutex, std::string& buffer) {
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

bool send_message(std::unique_ptr<zmq::socket_t>& socket, std::mutex& mutex, const ::google::protobuf::Message& message) {
    // assert((std::is_base_of<::google::protobuf::Message, T>::value));

    std::string buffer;
    if (message.SerializeToString(&buffer) == true) {
        return send_zmq(socket, mutex, buffer);
    } else {
        return false;
    }
}

bool recv_message(std::unique_ptr<zmq::socket_t>& socket, std::mutex& mutex, ::google::protobuf::Message& message) {
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
    m_pub_socket = std::make_unique<zmq::socket_t>(zmq_ctx, ZMQ_PUB);
    m_pub_socket->setsockopt(ZMQ_SNDHWM, ipc_ZMQ_SEND_QUEUE);
    m_pub_socket->setsockopt(ZMQ_LINGER, ipc_ZMQ_CLOSE_WAIT);

    pub_list_ = std::make_unique<SubscribeNodeList>();

    std::string server_pub_addr = splice("tcp://", ipc_SERVER_REP_ADDR, ipc_TOPIC_START_PORT);
    m_pub_socket->bind(server_pub_addr.c_str());
}

void PublisherImpl::PublisherOnline(const RoutingMessage &request)
{
    // notify
    LOG(INFO) << "id:" << request.node().client_id() << " topic:" << request.node().message_topic();

    // assign port
    std::string pub_addr;
    std::string server_pub_addr = splice("tcp://", ipc_SERVER_REP_ADDR, ipc_TOPIC_START_PORT);

    // reponse publisher addr, only one node
    std::string topic = request.node().message_topic();

    // boardcast publisher online
    RoutingMessage pub_message(request);
    pub_message.mutable_node()->set_socket_addr(pub_addr);
    send_message(m_pub_socket, m_pub_mutex, pub_message);

    // sync
    std::lock_guard<std::mutex> lock(m_topic_mutex);

    // append topic list
    auto pub_node = pub_list_->add_node_list();
    pub_node->set_node_type(ROUTING_NODE_PUB);
    pub_node->set_message_topic(topic);
    pub_node->set_socket_addr(request.node().socket_addr());
    pub_node->set_client_id(request.node().client_id());
    pub_node->set_client_ip(request.node().client_ip());
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

    auto node_list = pub_list_->mutable_node_list();
    auto iter = std::find_if(node_list->begin(), node_list->end(), [topic](const RoutingNode& item){
        return topic == item.message_topic();
    });

    if (iter != node_list->end())
    {
        node_list->erase(iter);
    }
}

////////////////////////////////////////////////////////////////////////////////
// subscriber
SubscriberImpl::SubscriberImpl(zmq::context_t& zmq_ctx)
{
    stop_ = false;
    m_pool_timeout = ipc_ZMQ_POOL_TIMEOUT;

    m_sub_socket = std::make_unique<zmq::socket_t>(zmq_ctx, ZMQ_SUB);
    m_sub_socket->setsockopt(ZMQ_SNDHWM, ipc_ZMQ_SEND_QUEUE);
    m_sub_socket->setsockopt(ZMQ_LINGER, ipc_ZMQ_CLOSE_WAIT);

    std::string server_sub_addr = splice("tcp://", ipc_SERVER_REP_ADDR, ipc_TOPIC_START_PORT);
    m_sub_socket->connect(server_sub_addr.c_str());

    m_sub_socket->setsockopt(ZMQ_SUBSCRIBE, "", 0); // subscribe all
}

SubscriberImpl::~SubscriberImpl()
{
    Stop();
}

bool SubscriberImpl::Connected() const
{
    return m_sub_socket != nullptr;
}


void SubscriberImpl::Run(SubscriberCallback&& callback)
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

                callback(message);
            }
        }
        catch (zmq::error_t &e)
        {
            LOG(ERROR) << (e.what());
        }
    }
}

void SubscriberImpl::Stop()
{
    stop_ = true;
}

////////////////////////////////////////////////////////////////////////////////
// routing server
ResponseImpl::ResponseImpl(zmq::context_t &zmq_ctx)
{
    // init status
    stop_ = false;

    // init zmq
    // static zmq::context_t zmq_ctx = zmq::context_t(1);

    m_rep_socket = std::make_unique<zmq::socket_t>(zmq_ctx, ZMQ_REP);
    m_rep_socket->setsockopt(ZMQ_SNDHWM, ipc_ZMQ_RECV_QUEUE);
    m_rep_socket->setsockopt(ZMQ_LINGER, ipc_ZMQ_CLOSE_WAIT);

    std::string server_rep_addr = splice("tcp://", ipc_SERVER_REP_ADDR, ipc_TOPIC_START_PORT);
    m_rep_socket->bind(server_rep_addr.c_str());
}

ResponseImpl::~ResponseImpl()
{
    this->Stop();
}

void ResponseImpl::Run(RequestCallback&& callback)
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

                callback(request);
            }
        }
        catch (zmq::error_t &e)
        {
            LOG(ERROR) << (e.what());
        }
    }
}

void ResponseImpl::Stop()
{
    stop_ = true;
}

////////////////////////////////////////////////////////////////////////////////
// class RequestImpl
RequestImpl::RequestImpl(zmq::context_t& zmq_ctx) 
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

    m_req_socket = std::make_unique<zmq::socket_t>(zmq_ctx, ZMQ_REQ);
    m_req_socket->setsockopt(ZMQ_SNDHWM, ipc_ZMQ_RECV_QUEUE);
    m_req_socket->setsockopt(ZMQ_LINGER, ipc_ZMQ_CLOSE_WAIT);

    std::string server_req_addr = splice("tcp://", ipc_SERVER_REP_ADDR, ipc_TOPIC_START_PORT);
    m_req_socket->connect(server_req_addr.c_str());
}

RequestImpl::~RequestImpl() 
{
}

bool RequestImpl::Connected() const 
{
    return m_req_socket != nullptr;
}

bool RequestImpl::Request(const RoutingMessage& message) 
{
    // request
    return send_message(m_req_socket, m_req_mutex, message);
}

bool RequestImpl::Response(RoutingMessage& response) 
{
    // response
    return recv_message(m_req_socket, m_req_mutex, response);
}

} // namespace zmqcopy 
} // namespace messages
} // namespace ipc
