// util messages service client interface
// zeromq implement

#pragma once
#include <string>
#include <map>
#include <set>
#include <memory>
#include <atomic>
#include <mutex>
#include <thread>
#include <google/protobuf/message.h>
#include "utils/util_threadpool.h"
#include "zmq_config.h"
#include "messages_callback.h"
#include "messages_routing.h"

// forward declaration for hiding zmq.hpp
namespace zmq {
    class socket_t;
}

namespace ipc {
namespace messages {

// forward declaration for hiding messages_protos.pb.h
class SubscribeNodeList;

// 用户回调函数调用模式
enum class CallbackMode {
    async, // 异步模式，每个消息单独线程执行，要求回调函数可重入/线程安全，默认为异步模式
    sync   // 同步模式，所有消息排队执行
};

using CallbackInterface = std::shared_ptr<VirtualMessageCallback>;
using CallbackFuncType = bool (*)(const std::string&, const MessageHeader&);
using ipc::util::ThreadPool;

class MessagesClient {

public:
    MessagesClient(const std::string& client_id, const std::string server_addr, const uint16_t server_port);
    ~MessagesClient();

    template<typename T>
    bool publish(const std::string& topic, const T& message) {
        assert(topic != "");
        assert((std::is_base_of<::google::protobuf::Message, T>::value));

        std::string buffer;
        ipc::util::Serializer<T> serializer;
        serializer.to_string(message, buffer);
        return this->publish<std::string>(topic, buffer);
    }

    template<typename T>
    bool subscribe(const std::string& topic, std::function<void(const T&)> callback_func) {
        assert(topic != "");
        assert((std::is_base_of<::google::protobuf::Message, T>::value));
        CallbackInterface callback_ptr = std::make_shared<MessageCallback1<T>>(callback_func);
        return register_subscription(topic, callback_ptr);
    }

    template<typename T, typename H>
    bool subscribe(const std::string& topic, std::function<void(const T&, const MessageHeader&)> callback_func) {
        assert(topic != "");
        assert((std::is_base_of<::google::protobuf::Message, T>::value));
        CallbackInterface callback_ptr = std::make_shared<MessageCallback2<T, MessageHeader>>(callback_func);
        return register_subscription(topic, callback_ptr);
    }

    bool register_subscription(const std::string& topic, CallbackInterface callback_ptr);
    bool unsubscribe(const std::string& topic);
    void spin();
    void stop();
    bool connected();

    // routing service interface
    bool node_list(SubscribeNodeList& node_list);

private:
    void publisher_online(const std::string& topic, const std::string& addr);
    void publisher_offline(const std::string& topic, const std::string& addr);
    void listen_thread();
    std::mutex& pub_mutex(const std::string& topic);

    // noncopyable
    MessagesClient(const MessagesClient&) = delete;
    MessagesClient& operator = (const MessagesClient&) = delete;

private:
    ////////////////////////////////////////////////////////////////////////////
    std::string m_id;
    std::atomic<bool> m_have_subscription;
    std::atomic<bool> m_exit_signal;
    std::thread m_listen_thread;
    CallbackMode m_callback_mode;
    std::shared_ptr<ThreadPool> m_thread_pool;

    ////////////////////////////////////////////////////////////////////////////
    // zmq settings
    int32_t m_send_queue;
    int32_t m_recv_queue;
    int32_t m_close_wait;
    int32_t m_pool_timeout;
    int32_t m_thread_workers;
    int32_t m_thread_queue_size;

    ////////////////////////////////////////////////////////////////////////////
    // routing protocol client
    std::shared_ptr<RoutingClient> m_routing_client;

    ////////////////////////////////////////////////////////////////////////////
    // topic -> publish socket, each publisher has a socket
    std::map<std::string, std::shared_ptr<zmq::socket_t>> m_pub_socket;
    // topic -> serial
    std::map<std::string, std::atomic<int64_t>> m_pub_serial;
    // topic -> mutex
    std::map<std::string, std::mutex> m_pub_mutex;

    ////////////////////////////////////////////////////////////////////////////
    // all subscriber share one socket
    std::shared_ptr<zmq::socket_t> m_sub_socket;
    std::mutex m_sub_mutex;
    std::set<std::string> m_sub_waiting;
    // topic -> callback_virtual_ptr
    std::map<std::string, CallbackInterface> m_subscribed;
};

template<>
bool MessagesClient::publish<std::string>(
    const std::string& topic, const std::string& message);

template<>
bool MessagesClient::subscribe<std::string>(
    const std::string& topic, std::function<void(const std::string&)> callback_func);

template<>
bool MessagesClient::subscribe<std::string, MessageHeader>(
    const std::string& topic, std::function<void(const std::string&, const MessageHeader&)> callback_func);

/*
C++11 14.7.3/16
GCC error: explicit specialization in non-namespace scope
http://www.open-std.org/jtc1/sc22/wg21/docs/standards
*/

} // namespace messages
} // namespace ipc
