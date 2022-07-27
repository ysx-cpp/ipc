// util messages service test/debug client
// support text messages only
// usage: ./messages_debuger <server> <port> <pub|sub> <topic> <pub message char> <pub message size> <pub interval(ms)>

#include <future>
#include <glog/logging.h>
#include "envelope.pb.h"
#include "sendassist.h"
#include "subscriber.h"
#include "utils.hpp"

using namespace ipc::messages;

void TestRequstReply(zmq::context_t &zmq_ctx)
{
    ipc::messages::RequestImpl client(zmq_ctx);
    client.Connect(g_config.server_req_addr());

    RoutingMessage req;
    RoutingMessage rsp;
    req.set_action(0);
    client.Request(req, rsp);
    LOGINFO << rsp.DebugString();
}

void TestSubscriber(zmq::context_t &zmq_ctx)
{
    ipc::messages::SubscriberImpl subscriber(zmq_ctx);
    subscriber.Connect(g_config.server_sub_addr(), g_config.topic_start_port());

    subscriber.Run([](const RoutingMessage& msg){
        LOGINFO << msg.DebugString();
    });
}

void Test(zmq::context_t &zmq_ctx)
{
    Subscriber subscriber(zmq_ctx, "");
    subscriber.Connect(g_config);
    RoutingMessage req;
    RoutingMessage rsp;
    req.set_action(MSG_ACTION_SUB_ONLINE);
    subscriber.Request(req, rsp);
    LOGINFO << rsp.DebugString();
    subscriber.Run();
}

int main(int argc, char** argv) 
{
    google::InitGoogleLogging("test_subscriber");
    FLAGS_log_dir = "log"; //log的目录
	FLAGS_logbufsecs = 0;//log在缓存中缓存的秒数，0表示不缓存，写就落地磁盘
	FLAGS_stderrthreshold = -1;//log级别大于该值的才同时输出到控制台
	FLAGS_colorlogtostderr = true;//log有颜色区分
	FLAGS_stop_logging_if_full_disk = true;//磁盘写满了就不写了

    assert(LoadConfig(argv[1], g_config));

    LOGINFO << g_config.DebugString();

    static zmq::context_t zmq_ctx = zmq::context_t(1);
    
    Test(zmq_ctx);

    return 0;
}
