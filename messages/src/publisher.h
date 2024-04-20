#pragma once
#include <mutex>
#include <memory>
#include "zmq.hpp"
#include "envelope.pb.h"
#include "config.pb.h"
#include "messageimpl.h"
#include "scheduler.h"

namespace zmq {
    class socket_t;
}

namespace ipc {
namespace messages {

class RoutingMessage;
class PublisherImpl;
class ReplyImpl;

class Publisher : public Scheduler
{
public:
    explicit Publisher(zmq::context_t& zmq_ctx, const std::string &topc);
    virtual ~Publisher();
    
    void Run();
    void Bind(const config::MessagesConfig& config);
    bool Publish(const RoutingMessage& message);
    virtual void RequestEvent(const RoutingMessage& message);
    virtual void SendResponse(const RoutingMessage& message);

protected:
    const std::string &topc_;
    std::unique_ptr<ReplyImpl> reply_;
    std::unique_ptr<PublisherImpl> publisher_;
};

} // namespace messages
} // namespace ipc
