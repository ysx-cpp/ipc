// util messages service test/debug client
// support text messages only
// usage: ./messages_debuger <server> <port> <pub|sub> <topic> <pub message char> <pub message size> <pub interval(ms)>

#include "messages_factory.h"

using namespace ipc::util;
using ipc::util::messages::ZmqMessagesClient;

void callback_echo(const std::string& message) {
    DEBUG_INFO("recv msg[%s]", message.c_str());
}

int main(int argc, char** argv) {
    if (argc < 5) {
        DEBUG_INFO("usage: ./messages_debuger <server> <port> <pub|sub> <topic> <pub message char> <pub message size> <pub interval(ms)>");
        return 0;
    }

    std::string server = argv[1];
    uint16_t port = safe_stoi(argv[2]);
    std::string action = argv[3];
    std::string topic = argv[4];

    ipc::util::messages::MessageFactory factory("zmq");
    ipc::util::messages::IClientNode* client = factory.create_client("util-messages-debuger", server, port);

    if (action == "pub") {
        if (argc < 8) {
            DEBUG_INFO("usage: ./messages_debuger <server> <port> <pub> <topic> <message char> <message size> <interval(ms)>");
            return 0;
        }

        std::string text = argv[5];
        int32_t size = safe_stoi(argv[6]);
        int32_t interval = safe_stoi(argv[7]);

        std::string message;
        message.resize(size, text[0]);

        while (true) {
            client->publish<std::string>(topic, message);
            sleep_ms(interval);
            DEBUG_INFO("send topic[%s] msg[%s]", topic.c_str(), message.c_str());
        }
    } else {
        client->subscribe<std::string>(topic, callback_echo);
        client->spin();
    }

    return 0;
}