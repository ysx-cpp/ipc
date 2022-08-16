#include "scheduler.h"
#include <future>
#include <functional>
#include <glog/logging.h>
#include "envelope.pb.h"
#include "messageimpl.h"
#include "utils.hpp"

namespace ipc {
namespace messages {

Scheduler::Scheduler(zmq::context_t& zmq_ctx) :
ProactiveSide(zmq_ctx, ""),
PassiveSide(zmq_ctx, "")
{
}

void Scheduler::Run()
{
}

void Scheduler::SubscribeEvent(const RoutingMessage& message)
{
    LOGINFO << message.DebugString();
}

void Scheduler::RequestEvent(const RoutingMessage& message)
{
    reply_->SendResponse(message);
    LOGINFO << message.DebugString();
}

void Scheduler::SubscribeOnline(const RoutingMessage& message)
{
    // sync
    std::lock_guard<std::mutex> lock(topic_mutex_);

    // append topic list
    auto sub_node = sub_list_->add_node_list();
    sub_node->CopyFrom(message.node());
}

void Scheduler::SubscribeOffline(const RoutingMessage& message)
{
    // delete topic node
    std::string topic = message.node().message_topic();

    auto node_list = sub_list_->mutable_node_list();
    auto iter = std::find_if(node_list->begin(), node_list->end(), [topic](const RoutingNode& item){
        return topic == item.message_topic();
    });

    if (iter != node_list->end())
    {
        node_list->erase(iter);
    }
}

} // namespace messages
} // namespace ipc
