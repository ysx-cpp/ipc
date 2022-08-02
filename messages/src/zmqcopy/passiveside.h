#pragma once
#include <mutex>
#include <memory>
#include "zmq.hpp"
#include "envelope.pb.h"
#include "messageimpl.h"

namespace zmq {
    class socket_t;
}

namespace ipc {
namespace messages {

class RoutingMessage;
class PublisherImpl;
class ReplyImpl;

class PassiveSide 
{
public:
    explicit PassiveSide(zmq::context_t& zmq_ctx, const std::string &topc);
    virtual ~PassiveSide();
    void Bind();
    bool Publish(const RoutingMessage& message);
    virtual void RequestEvent(const RoutingMessage& message);

protected:
    const std::string &topc_;
    std::unique_ptr<ReplyImpl> reply_;
    std::unique_ptr<PublisherImpl> publisher_;
};

} // namespace messages
} // namespace ipc
