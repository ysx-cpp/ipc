#include "messages_factory.h"

namespace ipc {
namespace messages {

IServerNode* MessageFactory::create_server(const std::string& name, uint16_t port) {
    return new ZmqMessagesServer(name, port);
}

IClientNode* MessageFactory::create_client(const std::string& name,
    const std::string& ip, uint16_t port) {
    return new ZmqMessagesClient(name, ip, port);
}

}
}
