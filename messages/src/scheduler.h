#pragma once
#include <mutex>
#include <memory>
#include "zmq.hpp"
#include "envelope.pb.h"
#include "proactiveside.h"
#include "passiveside.h"

namespace zmq {
    class socket_t;
}

namespace ipc {
namespace messages {

// forward declaration for hiding messages_protos.pb.h
class RoutingMessage;

class Scheduler : public ProactiveSide, public PassiveSide
{
public:
    explicit Scheduler(zmq::context_t& zmq_ctx);
    ~Scheduler() = default;
    void Run();
    void SubscribeEvent(const RoutingMessage& message) override;
    void RequestEvent(const RoutingMessage& message) override;

    void SubscribeOnline(const RoutingMessage& message);
    void SubscribeOffline(const RoutingMessage& message);

private:
    // topic + pub addr
    std::mutex topic_mutex_;
    std::unique_ptr<SubscribeNodeList> sub_list_;
};

} // namespace messages
} // namespace ipc
