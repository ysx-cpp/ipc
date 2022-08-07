#include "proactiveside.h"
#include <future>
#include <functional>
#include <glog/logging.h>
#include "envelope.pb.h"
#include "messageimpl.h"
#include "zmq_config.h"
#include "sendassist.h"

namespace ipc {
namespace messages {

ProactiveSide::ProactiveSide(zmq::context_t &zmq_ctx, const std::string &topc) :
topc_(topc),
request_(std::make_unique<RequestImpl>(zmq_ctx)),
subscriber_(std::make_unique<SubscriberImpl>(zmq_ctx))                                                                        
{
}

ProactiveSide::~ProactiveSide()
{
}

void ProactiveSide::Connect(const MessagesConfig& config)
{
    request_->Connect(config.server_rep_addr());
    subscriber_->Connect(config.server_pub_addr(), config.topic_start_port());

    auto sub_thread = std::async(std::launch::async, &SubscriberImpl::Run, subscriber_.get(),
                                 std::bind(&ProactiveSide::SubscribeEvent, this, std::placeholders::_1));

    sub_thread.wait();
}

bool ProactiveSide::Request(const RoutingMessage& request, RoutingMessage& response)
{
    return request_->Request(request, response);
}

void ProactiveSide::SubscribeEvent(const RoutingMessage &message)
{
    LOGINFO << message.DebugString();
}

} // namespace messages
} // namespace ipc
