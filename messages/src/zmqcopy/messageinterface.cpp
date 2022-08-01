#include "messageinterface.h"
#include <future>
#include <functional>
#include <glog/logging.h>

namespace ipc {
namespace messages {

ProactiveSide::ProactiveSide(zmq::context_t &zmq_ctx, const std::string &topc) :
request_(std::make_unique<RequestImpl>(zmq_ctx)),
subscriber_(std::make_unique<SubscriberImpl>(zmq_ctx))                                                                        
{
}

ProactiveSide::~ProactiveSide()
{
}

void ProactiveSide::Run()
{
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
    LOG(INFO) << message.DebugString();
}

///////////////////////////////////////////////////////////////////////////////////////

PassiveSide::PassiveSide(zmq::context_t &zmq_ctx, const std::string &topc) : 
reply_(std::make_unique<ReplyImpl>(zmq_ctx)),
publisher_(std::make_unique<PublisherImpl>(zmq_ctx))
{
}

PassiveSide::~PassiveSide()
{
}

void PassiveSide::Run()
{
    auto rep_thread = std::async(std::launch::async, &ReplyImpl::Run, reply_.get(),
                                 std::bind(&PassiveSide::RequestEvent, this, std::placeholders::_1));

    rep_thread.wait();
}

bool PassiveSide::Publish(const RoutingMessage& message)
{
    return publisher_->Publish(message);
}

void PassiveSide::RequestEvent(const RoutingMessage &message)
{
    LOG(INFO) << message.DebugString();
}

} // namespace messages
} // namespace ipc