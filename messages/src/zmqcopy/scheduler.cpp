#include "scheduler.h"
#include <future>
#include <functional>
#include <glog/logging.h>
#include "messages_protos.pb.h"
#include "messageimplement.h"

namespace ipc {
namespace messages {

// forward declaration for hiding messages_protos.pb.h
class RoutingMessage;
class SubscribeNodeList;

namespace zmqcopy {

Scheduler::Scheduler(zmq::context_t& zmq_ctx) :
publisher_(std::make_unique<PublisherImpl>(zmq_ctx)),
subscriber_(std::make_unique<SubscriberImpl>(zmq_ctx)),
response_(std::make_unique<ResponseImpl>(zmq_ctx)),
request_(std::make_unique<RequestImpl>(zmq_ctx)),
sub_list_(std::make_unique<SubscribeNodeList>())
{
}

void Scheduler::Run()
{
    auto sub_callback = std::bind(&Scheduler::SubscribeEvent, this, std::placeholders::_1);
    std::async(std::launch::async, &SubscriberImpl::Run, subscriber_.get(), sub_callback);

    auto rep_callback = std::bind(&Scheduler::RequestEvent, this, std::placeholders::_1);
    std::async(std::launch::async, &ResponseImpl::Run, response_.get(), rep_callback);
}

bool Scheduler::Publish(const RoutingMessage& message)
{
    return publisher_->Publish(message);
}

bool Scheduler::Request(const RoutingMessage& request, RoutingMessage& response)
{
    return request_->Request(request, response);
}

void Scheduler::SubscribeEvent(const RoutingMessage& message)
{

}

void Scheduler::RequestEvent(const RoutingMessage& message)
{

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

} // namespace zmqcopy 
} // namespace messages
} // namespace ipc