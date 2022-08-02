#include "passiveside.h"
#include <future>
#include <functional>
#include <glog/logging.h>
#include "envelope.pb.h"
#include "messageimpl.h"
#include "zmq_config.h"
#include "sendassist.h"

namespace ipc {
namespace messages {

PassiveSide::PassiveSide(zmq::context_t &zmq_ctx, const std::string &topc) : 
topc_(topc),
reply_(std::make_unique<ReplyImpl>(zmq_ctx)),
publisher_(std::make_unique<PublisherImpl>(zmq_ctx))
{
}

PassiveSide::~PassiveSide()
{
}

void PassiveSide::Bind()
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
