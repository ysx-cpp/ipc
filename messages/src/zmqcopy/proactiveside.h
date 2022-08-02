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
class SubscriberImpl;
class RequestImpl;

class ProactiveSide
{
public:
    explicit ProactiveSide(zmq::context_t& zmq_ctx, const std::string& topc);
    virtual ~ProactiveSide();
    void Connect();
    bool Request(const RoutingMessage& request, RoutingMessage& response);
    virtual void SubscribeEvent(const RoutingMessage& message);
    
protected:
    const std::string &topc_;
    std::unique_ptr<RequestImpl> request_;
    std::unique_ptr<SubscriberImpl> subscriber_;
};

} // namespace messages
} // namespace ipc
