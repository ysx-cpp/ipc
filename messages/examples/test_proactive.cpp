// util messages service test/debug client
// support text messages only
// usage: ./messages_debuger <server> <port> <pub|sub> <topic> <pub message char> <pub message size> <pub interval(ms)>

#include <glog/logging.h>
#include "zmqcopy/scheduler.h"
#include "envelope.pb.h"

using namespace ipc::messages;

int main(int argc, char** argv) 
{
    static zmq::context_t zmq_ctx = zmq::context_t(1);
    ipc::messages::Scheduler client(zmq_ctx);

    RoutingMessage msg;
    msg.set_action(100);
    client.Publish(msg);

    RoutingMessage req;
    RoutingMessage rsp;
    req.set_action(111);
    client.Request(req, rsp);

    LOG(INFO) << rsp.DebugString();

    return 0;
}
