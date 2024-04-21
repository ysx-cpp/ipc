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
publisher_(std::make_unique<PublisherImpl>(zmq_ctx)),
sub_list_(std::make_unique<SubscribeList>())
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

    MessageAction action = static_cast<MessageAction>(message.action());
    switch (action)
    {
    case MSG_ACTION_SUB_ONLINE:
        LOG(INFO)  << " online:" << message.DebugString();
        sub_list_->SubscribeOnline(message);
        break;
    case MSG_ACTION_SUB_OFFLINE:
        LOG(INFO)  << " offline:" << message.DebugString();
        sub_list_->SubscribeOffline(message);
        break;
    case MSG_ACTION_REQUEST:
    case MSG_ACTION_ECT:
        break;
    default:
        break;
    }
    reply_->SendResponse(message);
}

} // namespace messages
} // namespace ipc
