#include "subscriber.h"
#include <future>
#include <functional>
#include <glog/logging.h>
#include "envelope.pb.h"
#include "messageimpl.h"
#include "config.pb.h"
#include "sendassist.h"
#include "utils.hpp"

namespace ipc {
namespace messages {

Subscriber::Subscriber(zmq::context_t &zmq_ctx, const std::string &topc) :
topc_(topc),
request_(std::make_unique<RequestImpl>(zmq_ctx)),
subscriber_(std::make_unique<SubscriberImpl>(zmq_ctx))                                                                        
{
}

Subscriber::~Subscriber()
{
}

void Subscriber::Run()
{
    auto sub_thread = std::async(std::launch::async, &SubscriberImpl::Run, subscriber_.get(),
                                 std::bind(&Subscriber::SubscribeEvent, this, std::placeholders::_1));

    sub_thread.wait();
}

void Subscriber::Connect(const config::MessagesConfig& config)
{
    request_->Connect(config.server_req_addr());
    subscriber_->Connect(config.server_sub_addr(), config.topic_start_port());
}

bool Subscriber::Request(const RoutingMessage& request, RoutingMessage& response)
{
    return request_->Request(request, response);
}

void Subscriber::SubscribeEvent(const RoutingMessage &message)
{
    LOGINFO << message.DebugString();
}

} // namespace messages
} // namespace ipc
