#pragma once
#include <mutex>
#include <memory>
#include "zmq/zmq.hpp"

namespace zmq {
    class socket_t;
}

namespace ipc {
namespace messages {

// forward declaration for hiding messages_protos.pb.h
class RoutingMessage;
class SubscribeNodeList;

namespace zmqcopy {

class PublisherImpl;
class SubscriberImpl;
class ResponseImpl;
class RequestImpl;
class Scheduler
{
public:
    Scheduler(zmq::context_t& zmq_ctx);

    void Run();
    bool Publish(const RoutingMessage& message);
    bool Request(const RoutingMessage& request, RoutingMessage& response);
    void SubscribeEvent(const RoutingMessage& message);
    void RequestEvent(const RoutingMessage& message);

    void SubscribeOnline(const RoutingMessage& message);
    void SubscribeOffline(const RoutingMessage& message);

private:
    std::unique_ptr<PublisherImpl> publisher_;
    std::unique_ptr<SubscriberImpl> subscriber_;
    std::unique_ptr<ResponseImpl> response_;
    std::unique_ptr<RequestImpl> request_;

private:
    // topic + pub addr
    std::mutex topic_mutex_;
    std::unique_ptr<SubscribeNodeList> sub_list_;
};

} // namespace zmqcopy 
} // namespace messages
} // namespace ipc
