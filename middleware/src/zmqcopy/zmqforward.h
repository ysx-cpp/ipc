#pragma once
#include "messageimplement.h"

namespace ipc {
namespace messages {

// forward declaration for hiding messages_protos.pb.h
class RoutingMessage;
class SubscribeNodeList;

namespace zmqcopy {

class ZmqForward
{
public:
    ZmqForward();
    void SubscribeOnline(const RoutingMessage& message);
    void SubscribeOffline(const RoutingMessage& message);

private:

    // topic + pub addr
    std::mutex topic_mutex_;
    std::unique_ptr<SubscribeNodeList> sub_list_;
};

} // namespace zmqcopy 
} // namespace messages
} // namespace ipc
