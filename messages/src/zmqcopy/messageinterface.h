#pragma once
#include <memory>
#include "zmq.hpp"

namespace ipc {
namespace messages {

class RoutingMessage;

class ProactiveSide
{
public:
    ProactiveSide() = default;
    // virtual ~ProactiveSide() = 0;
    virtual bool Publish(const RoutingMessage& message) = 0;
    virtual bool Request(const RoutingMessage& request, RoutingMessage& response) = 0;
};

class PassiveSide
{
public:
    PassiveSide() = default;
    // virtual ~PassiveSide() = 0;
    virtual void Run() = 0;
    virtual void SubscribeEvent(const RoutingMessage& message) = 0;
    virtual void RequestEvent(const RoutingMessage& message) = 0;
};

static std::unique_ptr<ProactiveSide> CreateProactiveSide(zmq::context_t& zmq_ctx, const std::string &topc);

static std::unique_ptr<PassiveSide> CreatePassiveSide(zmq::context_t& zmq_ctx, const std::string &topc);

} // namespace messages
} // namespace ipc
