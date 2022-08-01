#pragma once
#include <memory>
#include "zmq.hpp"
#include "messageimpl.h"
#include "envelope.pb.h"

namespace ipc {
namespace messages {

class RoutingMessage;

class ProactiveSide
{
public:
    explicit ProactiveSide(zmq::context_t& zmq_ctx, const std::string& topc);
    virtual ~ProactiveSide();
    void Run();
    bool Request(const RoutingMessage& request, RoutingMessage& response);
    virtual void SubscribeEvent(const RoutingMessage& message);
    
protected:
    std::unique_ptr<RequestImpl> request_;
    std::unique_ptr<SubscriberImpl> subscriber_;
};

class PassiveSide 
{
public:
    explicit PassiveSide(zmq::context_t& zmq_ctx, const std::string &topc);
    virtual ~PassiveSide();
    void Run();
    bool Publish(const RoutingMessage& message);
    virtual void RequestEvent(const RoutingMessage& message);

protected:
    std::unique_ptr<ReplyImpl> reply_;
    std::unique_ptr<PublisherImpl> publisher_;
};

} // namespace messages
} // namespace ipc
