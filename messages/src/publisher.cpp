#include "publisher.h"
#include <future>
#include <functional>
#include <glog/logging.h>
#include "envelope.pb.h"
#include "config.pb.h"
#include "messageimpl.h"
#include "sendassist.h"

namespace ipc {
namespace messages {

Publisher::Publisher(zmq::context_t &zmq_ctx, const std::string &topc) : 
topc_(topc),
reply_(std::make_unique<ReplyImpl>(zmq_ctx)),
publisher_(std::make_unique<PublisherImpl>(zmq_ctx))
{
}

Publisher::~Publisher()
{
}

void Publisher::Run()
{
    auto rep_thread = std::async(std::launch::async, &ReplyImpl::Run, reply_.get(),
                                 std::bind(&Publisher::RequestEvent, this, std::placeholders::_1));

    rep_thread.wait();
}

void Publisher::Bind(const config::MessagesConfig& config)
{
    reply_->Bind(config.server_rep_addr());
    publisher_->Bind(config.server_pub_addr());
}

bool Publisher::Publish(const RoutingMessage& message)
{
    return publisher_->Publish(message);
}

void Publisher::RequestEvent(const RoutingMessage &message)
{
    LOG(INFO) << "request:" << message.DebugString();

    ActionType action = static_cast<ActionType>(message.action());
    switch (action)
    {
    case ActionType::ACTION_ONLINE:
        LOG(INFO)  << " online:" << message.DebugString();
        SubscribeOnline(message);
        break;
    case ActionType::ACTION_OFFLINE:
        LOG(INFO)  << " offline:" << message.DebugString();
        SubscribeOffline(message);
        break;
    case ActionType::ACTION_REQUEST:
    case ActionType::ACTION_ETC:
        break;
    default:
        break;
    }
    SendResponse(message);
}

void Publisher::SendResponse(const RoutingMessage& message)
{
    reply_->SendResponse(message);
}

} // namespace messages
} // namespace ipc
