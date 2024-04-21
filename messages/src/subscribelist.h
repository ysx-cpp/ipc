#pragma once
#include <mutex>
#include <memory>
#include "zmq.hpp"
#include "envelope.pb.h"

namespace zmq {
    class socket_t;
}

namespace ipc {
namespace messages {

// forward declaration for hiding messages_protos.pb.h
class RoutingMessage;

class SubscribeList
{
public:
    explicit SubscribeList();
    virtual ~SubscribeList() = default;

    void SubscribeOnline(const RoutingMessage& message);
    void SubscribeOffline(const RoutingMessage& message);
    const SubscribeNodeList &sub_nod_list() const {return *sub_nod_list_;}

private:
    // topic + pub addr
    std::mutex topic_mutex_;
    std::unique_ptr<SubscribeNodeList> sub_nod_list_;
};

} // namespace messages
} // namespace ipc
