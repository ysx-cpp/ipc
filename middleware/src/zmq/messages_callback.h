// util messages virtual-callback interface

#pragma once
#include <string>
#include <functional>
#include "utils/util_serializer.h"

namespace ipc {
namespace util {
namespace messages {

struct MessageHeader {
    std::string topic;
    std::string publisher;
    int64_t serial;
    int64_t send_time_ms;
    int64_t send_time_us; // µs
    int64_t recv_time_ms;
    int64_t recv_time_us; // µs
};

class VirtualMessageCallback {
public:
    virtual ~VirtualMessageCallback() {}
    virtual bool call(const std::string& payload, const MessageHeader& header) = 0;
};

// one parameter callback
template<typename T>
class MessageCallback1: public VirtualMessageCallback {

public:
    MessageCallback1() = delete;
    MessageCallback1(std::function<void(const T&)> func): m_func(func) {}
    virtual ~MessageCallback1() {}

    bool call(const std::string& payload, const MessageHeader& header) {
        T param;
        if (m_serializer.from_string(payload, param)) {
            m_func(param);
            return true;
        } else {
            return false;
        }
    }

private:
    ipc::util::Serializer<T> m_serializer;
    std::function<void(const T&)> m_func;
};

// two parameters callback
template<typename T, typename H>
class MessageCallback2: public VirtualMessageCallback {

public:
    MessageCallback2() = delete;
    MessageCallback2(std::function<void(const T&, const MessageHeader&)> func): m_func(func) {}
    virtual ~MessageCallback2() {}

    bool call(const std::string& payload, const MessageHeader& header) {
        T param;
        if (m_serializer.from_string(payload, param)) {
            m_func(param, header);
            return true;
        } else {
            return false;
        }
    }

private:
    ipc::util::Serializer<T> m_serializer;
    std::function<void(const T&, const MessageHeader&)> m_func;
};

} // namespace messages
} // namespace util
} // namespace ipc
