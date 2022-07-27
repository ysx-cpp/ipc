// util messages service test/debug client
// support text messages only
// usage: ./messages_debuger <server> <port> <pub|sub> <topic> <pub message char> <pub message size> <pub interval(ms)>

#include "envelope.pb.h"
#include "config.pb.h"
#include "sendassist.h"
#include "publisher.h"
#include "utils.hpp"

using namespace ipc::messages;
using namespace ipc::config;

int main(int argc, char** argv) 
{
    google::InitGoogleLogging("test_publisher");
    FLAGS_log_dir = "log"; //log的目录
	FLAGS_logbufsecs = 0;//log在缓存中缓存的秒数，0表示不缓存，写就落地磁盘
	FLAGS_stderrthreshold = -1;//log级别大于该值的才同时输出到控制台
	FLAGS_colorlogtostderr = true;//log有颜色区分
	FLAGS_stop_logging_if_full_disk = true;//磁盘写满了就不写了

    LOGINFO << g_config.DebugString();

    assert(LoadConfig(argv[1], g_config));

    static zmq::context_t zmq_ctx = zmq::context_t(1);
    ipc::messages::Publisher server(zmq_ctx, "");

    server.Bind(g_config);
    server.Run();

    return 0;
}
