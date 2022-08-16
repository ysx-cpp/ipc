// zeromq messages service procotol implement

#include "messageimpl.h"
#include <algorithm>
#include <chrono>
#include <glog/logging.h>
#include "utils.hpp"
#include "envelope.pb.h"
#include "sendassist.h"

namespace ipc {
namespace messages {

//////////////////////////////////////class PublisherImpl//////////////////////////////////////////
PublisherImpl::PublisherImpl(zmq::context_t &zmq_ctx)
{
    pub_socket_ = std::make_unique<zmq::socket_t>(zmq_ctx, ZMQ_PUB);
    // pub_socket_->setsockopt(ZMQ_SNDHWM, ipc_ZMQ_SEND_QUEUE);
    // pub_socket_->setsockopt(ZMQ_LINGER, ipc_ZMQ_CLOSE_WAIT);
    pub_socket_->set(zmq::sockopt::sndhwm, g_config.zmq_send_queue());
    pub_socket_->set(zmq::sockopt::linger, g_config.zmq_close_wait());
}

void PublisherImpl::Bind(const std::string& host)
{
    LOGINFO << "bind server_pub_addr:" << host;
    pub_socket_->bind(host.c_str());
}

bool PublisherImpl::Publish(const RoutingMessage &message)
{
    // notify
    LOGINFO << "id:" << message.node().client_id() << " topic:" << message.node().message_topic();
    
    // pub_socket_->send(zmq::str_buffer("A"), zmq::send_flags::sndmore);
    // pub_socket_->send(zmq::str_buffer("Message in A envelope"));
    Encode(message, buffer_);
    return SendMessage(pub_socket_, buffer_);
}

///////////////////////////////////////class subscriber/////////////////////////////////////////
SubscriberImpl::SubscriberImpl(zmq::context_t& zmq_ctx)
{
    stop_ = false;
    pool_timeout_ = std::chrono::milliseconds(g_config.zmq_pool_timeout());

    sub_socket_ = std::make_unique<zmq::socket_t>(zmq_ctx, ZMQ_SUB);
    // sub_socket_->setsockopt(ZMQ_REVHWM, ipc_ZMQ_RECV_QUEUE);
    // sub_socket_->setsockopt(ZMQ_LINGER, ipc_ZMQ_CLOSE_WAIT);
    sub_socket_->set(zmq::sockopt::rcvhwm, g_config.zmq_recv_queue());
    sub_socket_->set(zmq::sockopt::linger, g_config.zmq_close_wait());
}

SubscriberImpl::~SubscriberImpl()
{
    Stop();
}

bool SubscriberImpl::Connect(const std::string& host, const std::string& topc)
{
    sub_socket_->connect(host.c_str());
    LOGINFO << "connect server_pub_addr:" << host;

    // sub_socket_->setsockopt(ZMQ_SUBSCRIBE, "", 0); // subscribe all
    // sub_socket_->set(zmq::sockopt::subscribe, ""); // subscribe all
    sub_socket_->set(zmq::sockopt::subscribe, topc);

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
                if (!RecvMessage(sub_socket_, buffer_))
                {
                    continue;
                }

                RoutingMessage message;
                if (Decode(buffer_, message))
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

///////////////////////////////////////class ReplyImpl/////////////////////////////////////////
ReplyImpl::ReplyImpl(zmq::context_t &zmq_ctx)
{
    // init status
    stop_ = false;
    pool_timeout_ = std::chrono::milliseconds(g_config.zmq_pool_timeout());

    rep_socket_ = std::make_unique<zmq::socket_t>(zmq_ctx, ZMQ_REP);
    // rep_socket_->setsockopt(ZMQ_SNDHWM, ipc_ZMQ_RECV_QUEUE);
    // rep_socket_->setsockopt(ZMQ_LINGER, ipc_ZMQ_CLOSE_WAIT);
    rep_socket_->set(zmq::sockopt::sndhwm, g_config.zmq_recv_queue());
    rep_socket_->set(zmq::sockopt::linger, g_config.zmq_close_wait());
}

ReplyImpl::~ReplyImpl()
{
    this->Stop();
}

void ReplyImpl::Bind(const std::string& host)
{
    LOGINFO << "bind server_rep_addr:" << host;
    rep_socket_->bind(host.c_str());
}

void ReplyImpl::Run(RequestCallback&& callback)
{
    zmq::pollitem_t zmq_pool_item = {*rep_socket_, 0, ZMQ_POLLIN, 0};
    while (!stop_)
    {
        try
        {
            int rc = zmq::poll(&zmq_pool_item, 1, pool_timeout_);
            if (rc < 0)
                continue;

            if (zmq_pool_item.revents & ZMQ_POLLIN)
            {
                if (!RecvMessage(rep_socket_, buffer_))
                {
                    LOGERROR << "RecvMessage ERROR";
                    continue;
                }

                RoutingMessage message;
                if (Decode(buffer_, message))
                    callback(message);

                // client ip
                // zmq::message_t message;
                // if (rep_socket_->recv(&message, ZMQ_DONTWAIT) == false)
                // {
                //     continue;
                // }
                //request.mutable_node()->set_client_ip(message.gets("Peer-Address"));
            }
        }
        catch (zmq::error_t &e)
        {
            LOG(ERROR) << (e.what());
        }
    }
}

bool ReplyImpl::SendResponse(const RoutingMessage& message)
{
    // notify
    LOGINFO << "id:" << message.node().client_id() << " topic:" << message.node().message_topic();
    
    // pub_socket_->send(zmq::str_buffer("A"), zmq::send_flags::sndmore);
    // pub_socket_->send(zmq::str_buffer("Message in A envelope"));
    StreamBuffer buffer;
    Encode(message, buffer);
    return SendMessage(rep_socket_, buffer);
}

void ReplyImpl::Stop()
{
    stop_ = true;
}

///////////////////////////////////////class RequestImpl/////////////////////////////////////////
RequestImpl::RequestImpl(zmq::context_t& zmq_ctx) 
{
    req_socket_ = std::make_unique<zmq::socket_t>(zmq_ctx, ZMQ_REQ);
    // req_socket_->setsockopt(ZMQ_SNDHWM, ipc_ZMQ_SEND_QUEUE);
    // req_socket_->setsockopt(ZMQ_LINGER, ipc_ZMQ_CLOSE_WAIT);
    req_socket_->set(zmq::sockopt::sndhwm, g_config.zmq_send_queue());
    req_socket_->set(zmq::sockopt::linger, g_config.zmq_close_wait());
}

bool RequestImpl::Connect(const std::string& host)
{
    LOGINFO << "connect req_addr:" << host;
    req_socket_->connect(host.c_str());
    return req_socket_ != nullptr;
}

bool RequestImpl::Request(const RoutingMessage& request, RoutingMessage& response) 
{
    // request
    if (!Encode(request, send_buffer_))
        return false;
    
    if (!SendMessage(req_socket_, send_buffer_))
        return false;

    // response
    if (!RecvMessage(req_socket_, recv_buffer_))
        return false;

    return Decode(recv_buffer_, response);
}

///////////////////////////////////////class RouterImpl/////////////////////////////////////////
RouterImpl::RouterImpl(zmq::context_t& zmq_ctx)
{
    frontend_router_socket_ = std::make_unique<zmq::socket_t>(zmq_ctx, ZMQ_REQ);
    backend_router_socket_ = std::make_unique<zmq::socket_t>(zmq_ctx, ZMQ_REQ);

    frontend_router_socket_->bind("ipc://frontend.ipc");
    backend_router_socket_->bind("ipc://backend.ipc");
}

void RouterImpl::Run()
{
    // try
    // {
    //     while (!stop_)
    //     {
    //         zmq::pollitem_t zmq_pool_item[] = {
    //             {*frontend_router_socket_, 0, ZMQ_POLLIN, 0},
    //             {*backend_router_socket_, 0, ZMQ_POLLIN, 0}};

    //         int available_workers = 2;

    //         int rc = zmq::poll(&zmq_pool_item, available_workers, pool_timeout_);
    //         if (rc < 0)
    //         {
    //             continue;
    //         }

    //         if (zmq_pool_item[0].revents & ZMQ_POLLIN)
    //         {
    //             RoutingMessage message;
    //             if (recv_message(frontend_router_socket_, frontend_route_mutex_, message) == false)
    //             {
    //                 continue;
    //             }

    //             callback(message);
    //         }
    //         if (zmq_pool_item[1].revents & ZMQ_POLLIN)
    //         {
    //             RoutingMessage message;
    //             if (recv_message(backend_router_socket_, backend_route_mutex_, message) == false)
    //             {
    //                 continue;
    //             }

    //             callback(message);
    //         }
    //     }
    // }
    // catch (zmq::error_t &e)
    // {
    //     LOG(ERROR) << (e.what());
    // }
}

} // namespace messages
} // namespace ipc
