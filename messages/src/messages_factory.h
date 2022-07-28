#ifndef RECORDER_MESSAGES_FACTORY_H
#define RECORDER_MESSAGES_FACTORY_H

#include "zmq/messages_interface.h"

namespace ipc {
namespace messages {

class MessageFactory {
public:
    MessageFactory() {
        _type = "zmq";
    }
    MessageFactory(const std::string type) : _type(type) {}
    ~MessageFactory() {}
    IServerNode* create_server(const std::string& name, uint16_t port);
    IClientNode* create_client(const std::string& name, const std::string& ip, uint16_t port);
private:
    std::string _type;
};

} //messages
} //ipc
#endif //RECORDER_MESSAGES_FACTORY_H
