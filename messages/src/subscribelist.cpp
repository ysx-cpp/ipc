#include "subscribelist.h"
#include <future>
#include <functional>
#include <glog/logging.h>
#include "envelope.pb.h"
#include "messageimpl.h"
#include "utils.hpp"

namespace ipc {
namespace messages {

SubscribeList::SubscribeList() : 
sub_nod_list_(std::make_unique<SubscribeNodeList>())
{
}

void SubscribeList::SubscribeOnline(const RoutingMessage& message)
{
    // sync
    std::lock_guard<std::mutex> lock(topic_mutex_);

    const std::string &topic = message.node().message_topic();

    auto node_list = sub_nod_list_->mutable_node_list();
    if (node_list != nullptr)
    {
        auto iter = std::find_if(node_list->begin(), node_list->end(), [topic](const RoutingNode &item)
                                 { return topic == item.message_topic(); });
        // append topic list
        if (iter != node_list->end())
        {
            std::cerr << __PRETTY_FUNCTION__ << "error topic:" << topic << std::endl;
            return;
        }
    }
    auto sub_node = sub_nod_list_->add_node_list();
    sub_node->CopyFrom(message.node());
}

void SubscribeList::SubscribeOffline(const RoutingMessage& message)
{
    // sync
    std::lock_guard<std::mutex> lock(topic_mutex_);

    // delete topic node
    std::string topic = message.node().message_topic();

    auto node_list = sub_nod_list_->mutable_node_list();
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
