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
    pub_socket_ = std::make_unique<zmq::socket_t>(zmq_ctx, ZMQ_PUB);
    pub_socket_->setsockopt(ZMQ_SNDHWM, ipc_ZMQ_SEND_QUEUE);
    pub_socket_->setsockopt(ZMQ_LINGER, ipc_ZMQ_CLOSE_WAIT);

    std::string server_pub_addr = splice("tcp://", ipc_SERVER_REP_ADDR, ipc_TOPIC_START_PORT);
    pub_socket_->bind(server_pub_addr.c_str());
}

bool PublisherImpl::Publish(const RoutingMessage &message)
{
    // notify
    LOG(INFO) << "id:" << message.node().client_id() << " topic:" << message.node().message_topic();
    return send_message(pub_socket_, pub_mutex_, message);
}

////////////////////////////////////////////////////////////////////////////////
// subscriber
SubscriberImpl::SubscriberImpl(zmq::context_t& zmq_ctx)
{
    stop_ = false;
    pool_timeout_ = ipc_ZMQ_POOL_TIMEOUT;

    sub_socket_ = std::make_unique<zmq::socket_t>(zmq_ctx, ZMQ_SUB);
    sub_socket_->setsockopt(ZMQ_SNDHWM, ipc_ZMQ_SEND_QUEUE);
    sub_socket_->setsockopt(ZMQ_LINGER, ipc_ZMQ_CLOSE_WAIT);

    std::string server_sub_addr = splice("tcp://", ipc_SERVER_REP_ADDR, ipc_TOPIC_START_PORT);
    sub_socket_->connect(server_sub_addr.c_str());

    sub_socket_->setsockopt(ZMQ_SUBSCRIBE, "", 0); // subscribe all
}

SubscriberImpl::~SubscriberImpl()
{
    Stop();
}

bool SubscriberImpl::Connected() const
{
    return sub_socket_ != nullptr;
}

void SubscriberImpl::Run(SubscribeCallback &&callback)
{
    try
    {
        while (!stop_)
        {
            zmq::pollitem_t zmq_pool_item = {*sub_socket_, 0, ZMQ_POLLIN, 0};
            int rc = zmq::poll(&zmq_pool_item, 1, pool_timeout_);
            if (rc < 0)
            {
                continue;
            }

            if (zmq_pool_item.revents & ZMQ_POLLIN)
            {
                RoutingMessage message;
                if (recv_message(sub_socket_, sub_mutex_, message) == false)
                {
                    continue;
                }

                callback(message);
            }
        }
    }
    catch (zmq::error_t &e)
    {
        LOG(ERROR) << (e.what());
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

    rep_socket_ = std::make_unique<zmq::socket_t>(zmq_ctx, ZMQ_REP);
    rep_socket_->setsockopt(ZMQ_SNDHWM, ipc_ZMQ_RECV_QUEUE);
    rep_socket_->setsockopt(ZMQ_LINGER, ipc_ZMQ_CLOSE_WAIT);

    std::string server_rep_addr = splice("tcp://", ipc_SERVER_REP_ADDR, ipc_TOPIC_START_PORT);
    rep_socket_->bind(server_rep_addr.c_str());
}

ResponseImpl::~ResponseImpl()
{
    this->Stop();
}

void ResponseImpl::Run(RequestCallback&& callback)
{
    zmq::pollitem_t zmq_pool_item = {*rep_socket_, 0, ZMQ_POLLIN, 0};
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
                    std::lock_guard<std::mutex> lock(rep_mutex_);
                    if (rep_socket_->recv(&message, ZMQ_DONTWAIT) == false)
                    {
                        continue;
                    }
                }
                // deserialization
                std::string buffer = std::string(static_cast<char *>(message.data()), message.size());
                RoutingMessage request;
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

    req_socket_ = std::make_unique<zmq::socket_t>(zmq_ctx, ZMQ_REQ);
    req_socket_->setsockopt(ZMQ_SNDHWM, ipc_ZMQ_RECV_QUEUE);
    req_socket_->setsockopt(ZMQ_LINGER, ipc_ZMQ_CLOSE_WAIT);

    std::string server_req_addr = splice("tcp://", ipc_SERVER_REP_ADDR, ipc_TOPIC_START_PORT);
    req_socket_->connect(server_req_addr.c_str());
}

bool RequestImpl::Connected() const 
{
    return req_socket_ != nullptr;
}

bool RequestImpl::Request(const RoutingMessage& request, RoutingMessage& response) 
{
    // request
    auto ret = send_message(req_socket_, req_mutex_, request);

    // response
    ret = recv_message(req_socket_, req_mutex_, response);

    return ret;
}

////////////////////////////////////////////////////////////////////////////////
//class RouterImpl
RouterImpl::RouterImpl(zmq::context_t& zmq_ctx)
{
    frontend_router_socket_ = std::make_unique<zmq::socket_t>(zmq_ctx, ZMQ_REQ);
    backend_router_socket_ = std::make_unique<zmq::socket_t>(zmq_ctx, ZMQ_REQ);

    frontend_router_socket_->bind("ipc://frontend.ipc");
    backend_router_socket_->bind("ipc://backend.ipc");
}

void RouterImpl::Run()
{
    try
    {
        while (!stop_)
        {
            zmq::pollitem_t zmq_pool_item[] = {
                {*frontend_router_socket_, 0, ZMQ_POLLIN, 0},
                {*backend_router_socket_, 0, ZMQ_POLLIN, 0}};

            int available_workers = 2;

            int rc = zmq::poll(&zmq_pool_item, available_workers, pool_timeout_);
            if (rc < 0)
            {
                continue;
            }

            if (zmq_pool_item[0].revents & ZMQ_POLLIN)
            {
                RoutingMessage message;
                if (recv_message(frontend_router_socket_, frontend_route_mutex_, message) == false)
                {
                    continue;
                }

                callback(message);
            }
            if (zmq_pool_item[1].revents & ZMQ_POLLIN)
            {
                RoutingMessage message;
                if (recv_message(backend_router_socket_, backend_route_mutex_, message) == false)
                {
                    continue;
                }

                callback(message);
            }
        }
    }
    catch (zmq::error_t &e)
    {
        LOG(ERROR) << (e.what());
    }
}

} // namespace zmqcopy 
} // namespace messages
} // namespace ipc
