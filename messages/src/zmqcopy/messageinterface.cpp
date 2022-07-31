#include "messageinterface.h"
#include <future>
#include <functional>
#include <glog/logging.h>
#include "messageimpl.h"
#include "envelope.pb.h"

namespace ipc {
namespace messages {
    
class ProactiveSideImpl : public ProactiveSide
{
public:
    explicit ProactiveSideImpl(zmq::context_t& zmq_ctx, const std::string &topc)
    {

    }
    virtual ~ProactiveSideImpl()
    {
    }

    bool Publish(const RoutingMessage& message)
    {
        return publisher_->Publish(message);
    }

    bool Request(const RoutingMessage& request, RoutingMessage& response)
    {
        return request_->Request(request, response);
    }

protected:
    std::unique_ptr<RequestImpl> request_;
    std::unique_ptr<PublisherImpl> publisher_;
};

class PassiveSideImpl : public PassiveSide
{
public:
    explicit PassiveSideImpl(zmq::context_t& zmq_ctx, const std::string& topc) :
    subscriber_(std::make_unique<SubscriberImpl>(zmq_ctx)),
    reply_(std::make_unique<ReplyImpl>(zmq_ctx))
    {
    }

    virtual ~PassiveSideImpl()
    {
    }

    void Run()
    {
        auto sub_thread = std::async(std::launch::async, &SubscriberImpl::Run, subscriber_.get(),
                                 std::bind(&PassiveSideImpl::SubscribeEvent, this, std::placeholders::_1));

        auto rep_thread = std::async(std::launch::async, &ReplyImpl::Run, reply_.get(),
                                     std::bind(&PassiveSideImpl::RequestEvent, this, std::placeholders::_1));

        sub_thread.wait();
        rep_thread.wait();
    }

    void SubscribeEvent(const RoutingMessage& message)
    {
        LOG(INFO) << message.DebugString();
    }

    void RequestEvent(const RoutingMessage& message)
    {
        LOG(INFO) << message.DebugString();
    }

protected:
    std::unique_ptr<ReplyImpl> reply_;
    std::unique_ptr<SubscriberImpl> subscriber_;
};

std::unique_ptr<ProactiveSide> CreateProactiveSide(zmq::context_t& zmq_ctx, const std::string& topc)
{
    return std::make_unique<ProactiveSideImpl>(zmq_ctx, topc);
}

std::unique_ptr<PassiveSide> CreatePassiveSide(zmq::context_t& zmq_ctx, const std::string& topc)
{
    return std::make_unique<PassiveSideImpl>(zmq_ctx, topc);
}

} // namespace messages
} // namespace ipc