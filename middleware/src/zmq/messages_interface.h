#pragma once
#include <string>
#include <functional>
//#include "common/ipc_common.h"
//#include "ipc_messages.h"
#include "messages_client.h"

namespace ipc {
namespace util {
namespace messages {

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
        ipc::util::common::Serializer<T> serializer;
        serializer.to_string(message, buffer);
        return publish(topic, buffer);
    };

    template<typename T>
    bool publish(const std::string& topic, std::shared_ptr<T>& message) {
        assert(topic != "");
        //assert((std::is_base_of<::google::protobuf::Message, T>::value));

        std::string buffer;
        ipc::util::common::Serializer<T> serializer;
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

//////////////////////////////////////////////////////////////////////////////////
class ZmqMessagesClient : public IClientNode {
public:
    ZmqMessagesClient(const std::string& client_id);
    ZmqMessagesClient(const std::string& client_id, const std::string& server, uint16_t port);

    bool start();
    void spin();
    void stop();
    bool connected();

    bool publish(const std::string& topic, const std::string& message);
    bool publish(const std::string& topic, const std::shared_ptr<std::string>& message);
    bool subscribe(const std::string& topic, std::function<void(const std::string&)> callback_func);
    bool register_subscription(const std::string& topic, CallbackInterface callback);
/*

    template<typename T>
    bool publish(const std::string& topic, const T& message) {
        return m_client_ptr->publish<T>(topic, message);
    };

    template<typename T>
    bool publish(const std::string& topic, std::shared_ptr<T>& message) {
        return m_client_ptr->publish<T>(topic, *message);
    };

    template<typename T>
    bool subscribe(const std::string& topic, std::function<void(const T&)> callback_func) {
        return m_client_ptr->subscribe<T>(topic, callback_func);
    }
*/

private:
    std::atomic<bool> m_started;
    std::string m_id;
    std::string m_addr;
    uint16_t m_port;
    std::shared_ptr<ipc::util::messages::MessagesClient> m_client_ptr;
};

//////////////////////////////////////////////////////////////////////////////////
//server node interface
class IServerNode {
public:
    virtual bool start() = 0;
    virtual void spin() = 0;
    virtual void stop() = 0;
};

//////////////////////////////////////////////////////////////////////////////////
class ZmqMessagesServer : public IServerNode {
public:
    ZmqMessagesServer();
    ZmqMessagesServer(std::string name, uint16_t port);

    bool start();
    void spin() {}
    void stop();

private:
    std::string m_name;
    uint16_t m_port;
    std::shared_ptr<ipc::util::messages::RoutingServer> m_server_ptr;
};

}
}
}
