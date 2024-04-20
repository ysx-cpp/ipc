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

enum class ActionType
{
    ACTION_ONLINE,     //online
    ACTION_OFFLINE,    //offline
    ACTION_REQUEST,    //request
    ACTION_ETC,        // etc
};

// forward declaration for hiding messages_protos.pb.h
class RoutingMessage;

class Scheduler
{
public:
    explicit Scheduler();
    virtual ~Scheduler() = default;

    void SubscribeOnline(const RoutingMessage& message);
    void SubscribeOffline(const RoutingMessage& message);

private:
    // topic + pub addr
    std::mutex topic_mutex_;
    std::unique_ptr<SubscribeNodeList> sub_list_;
};

} // namespace messages
} // namespace ipc
