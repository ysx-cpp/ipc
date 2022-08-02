#include "messageinterface.h"
#include <future>
#include <functional>
#include <glog/logging.h>
#include "zmq_config.h"

namespace ipc {
namespace messages {

std::string ParseHost(const std::string& protocol, const std::string& ip, int port)
{
    std::string addr;
    addr.append(protocol).append("://").append(ip).append(":").append(std::to_string(port));
    return addr;
}

ProactiveSide::ProactiveSide(zmq::context_t &zmq_ctx, const std::string &topc) :
topc_(topc),
request_(std::make_unique<RequestImpl>(zmq_ctx)),
subscriber_(std::make_unique<SubscriberImpl>(zmq_ctx))                                                                        
{
}

ProactiveSide::~ProactiveSide()
{
}

void ProactiveSide::Run()
{
    std::string server_req_addr = ParseHost("tcp", ipc_SERVER_REQ_ADDR, ipc_SERVER_REP_PORT);
    request_->Connect(server_req_addr);
    subscriber_->Connect(server_req_addr, topc_);

    auto sub_thread = std::async(std::launch::async, &SubscriberImpl::Run, subscriber_.get(),
                                 std::bind(&ProactiveSide::SubscribeEvent, this, std::placeholders::_1));

    sub_thread.wait();
}

bool ProactiveSide::Request(const RoutingMessage& request, RoutingMessage& response)
{
    return request_->Request(request, response);
}

void ProactiveSide::SubscribeEvent(const RoutingMessage &message)
{
    LOG(INFO) << message.DebugString();
}

///////////////////////////////////////////////////////////////////////////////////////

PassiveSide::PassiveSide(zmq::context_t &zmq_ctx, const std::string &topc) : 
topc_(topc),
reply_(std::make_unique<ReplyImpl>(zmq_ctx)),
publisher_(std::make_unique<PublisherImpl>(zmq_ctx))
{
}

PassiveSide::~PassiveSide()
{
}

void PassiveSide::Run()
{
    std::string server_rep_addr = ParseHost("tcp", ipc_SERVER_REP_ADDR, ipc_SERVER_REP_PORT);
    reply_->Bind(server_rep_addr);

    std::string server_pub_addr = ParseHost("tcp", ipc_SERVER_SUB_ADDR, ipc_SERVER_PUB_PORT);
    publisher_->Bind(server_pub_addr);

    auto rep_thread = std::async(std::launch::async, &ReplyImpl::Run, reply_.get(),
                                 std::bind(&PassiveSide::RequestEvent, this, std::placeholders::_1));

    rep_thread.wait();
}

bool PassiveSide::Publish(const RoutingMessage& message)
{
    return publisher_->Publish(message);
}

void PassiveSide::RequestEvent(const RoutingMessage &message)
{
    reply_->SendResponse(message);
    LOG(INFO) << message.DebugString();
}

} // namespace messages
} // namespace ipc