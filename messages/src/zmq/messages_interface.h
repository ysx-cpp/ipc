#pragma once
#include <string>
#include <functional>
//#include "common/ipc_common.h"
//#include "ipc_messages.h"
#include "messages_client.h"

namespace ipc {
namespace messages {

//////////////////////////////////////////////////////////////////////////////////
//server node interface
class IServerNode {
public:
    virtual bool start() = 0;
    virtual void spin() = 0;
    virtual void stop() = 0;
};

IServerNode* CreateServerNode(const std::string& name, uint16_t port);

//////////////////////////////////////////////////////////////////////////////////
class IClientNode {
public:
    virtual ~IClientNode() {}

    virtual bool start() = 0;
    virtual void spin() = 0;
    virtual void stop() = 0;
    virtual bool connected() = 0;

    virtual bool publish(const std::string& topic, const std::string& content) = 0;
    virtual bool publish(const std::string& topic, const std::shared_ptr<std::string>& message) = 0;
    virtual bool subscribe(const std::string& topic, std::function<void(const std::string&)> callback_func) = 0;
    virtual bool register_subscription(const std::string& topic, CallbackInterface callback) = 0;

    template<typename T>
    bool publish(const std::string& topic, const T& message) {
        assert(topic != "");
        //assert((std::is_base_of<::google::protobuf::Message, T>::value));

        std::string buffer;
        ipc::util::Serializer<T> serializer;
        serializer.to_string(message, buffer);
        return publish(topic, buffer);
    };

    template<typename T>
    bool publish(const std::string& topic, std::shared_ptr<T>& message) {
        assert(topic != "");
        //assert((std::is_base_of<::google::protobuf::Message, T>::value));

        std::string buffer;
        ipc::util::Serializer<T> serializer;
        serializer.to_string(*message, buffer);
        return publish(topic, buffer);
    };

    template<typename T>
    bool subscribe(const std::string& topic, std::function<void(const T&)> callback_func) {
        assert(topic != "");
        //assert((std::is_base_of<::google::protobuf::Message, T>::value));

        CallbackInterface callback_ptr = std::make_shared<MessageCallback1<T>>(callback_func);
        return register_subscription(topic, callback_ptr);
    }
};

IClientNode* CreateClientNode(const std::string& name,
    const std::string& ip, uint16_t port);

}
}
