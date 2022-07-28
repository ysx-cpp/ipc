#include "zmqforward.h"
#include <glog/logging.h>
#include "messages_protos.pb.h"

namespace ipc {
namespace messages {

// forward declaration for hiding messages_protos.pb.h
class RoutingMessage;
class SubscribeNodeList;

namespace zmqcopy {

ZmqForward::ZmqForward()
{
    sub_list_ = std::make_unique<SubscribeNodeList>();
}

void ZmqForward::SubscribeOnline(const RoutingMessage& message)
{
    // sync
    std::lock_guard<std::mutex> lock(topic_mutex_);

    // append topic list
    auto sub_node = sub_list_->add_node_list();
    sub_node->CopyFrom(message.node());
}

void ZmqForward::SubscribeOffline(const RoutingMessage& message)
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
