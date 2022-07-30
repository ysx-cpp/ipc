// util messages service test/debug client
// support text messages only
// usage: ./messages_debuger <server> <port> <pub|sub> <topic> <pub message char> <pub message size> <pub interval(ms)>

#include "zmqcopy/scheduler.h"

int main(int argc, char** argv) 
{
    static zmq::context_t zmq_ctx = zmq::context_t(1);
    ipc::messages::Scheduler server(zmq_ctx);
    // ipc::messages::Scheduler client(zmq_ctx);

    server.Run();

    return 0;
}