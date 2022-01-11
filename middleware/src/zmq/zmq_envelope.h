// zeromq package/unpackage & send/recv interface

#pragma once
//#include <cstdint>
#include <string>
#include <mutex>
#include "utils/util_serializer.h"
#include "utils/util_debug.h"
#include "utils/util_time.h"

// forward declaration for hiding zmq.hpp
namespace zmq {
    class socket_t;
}

namespace ipc {
namespace util {
namespace messages {

using namespace ipc::util::common;

////////////////////////////////////////////////////////////////////////////////
// send & recv utility

bool send_zmq(std::shared_ptr<zmq::socket_t>& socket, std::mutex& mutex, const std::string& buffer);
bool recv_zmq(std::shared_ptr<zmq::socket_t>& socket, std::mutex& mutex, std::string& buffer);

template<typename T>
bool send_message(std::shared_ptr<zmq::socket_t>& socket, std::mutex& mutex, const T& message) {
    assert((std::is_base_of<::google::protobuf::Message, T>::value));

    std::string buffer;
    ipc::util::common::Serializer<T> serializer;
    if (serializer.to_string(message, buffer) == true) {
        return send_zmq(socket, mutex, buffer);
    } else {
        return false;
    }
}

template<typename T>
bool recv_message(std::shared_ptr<zmq::socket_t>& socket, std::mutex& mutex, T& message) {
    assert((std::is_base_of<::google::protobuf::Message, T>::value));

    std::string buffer;
    if (recv_zmq(socket, mutex, buffer) == false) {
        return false;
    }
    ipc::util::common::Serializer<T> serializer;
    return serializer.from_string(buffer, message);
}

////////////////////////////////////////////////////////////////////////////////
class MessageEnvelope {

public:
    MessageEnvelope(const std::string& topic, const std::string& publisher,
        const std::string& payload, const int64_t serial);
    MessageEnvelope() {};
    ~MessageEnvelope() {};

    // setter
    void fill(const std::string& topic, const std::string& publisher,
        const std::string& payload, const int64_t serial);

    // getter
    std::string topic() {
        return m_topic;
    }

    std::string publisher() {
        return m_publisher;
    }

    std::string payload() {
        return m_payload;
    }

    int64_t serial() {
        return m_serial;
    }

    int64_t send_time_ms() {
        return m_send_time_ms;
    }

    int64_t send_time_us() {
        return m_send_time_us;
    }

    int64_t recv_time_ms() {
        return m_recv_time_ms;
    }

    int64_t recv_time_us() {
        return m_recv_time_us;
    }

    // send & recv
    bool send(zmq::socket_t& zmq_socket);
    bool recv(zmq::socket_t& zmq_socket);

private:
    std::string m_topic;
    std::string m_publisher;
    std::string m_payload;
    int64_t m_serial;
    int64_t m_send_time_ms;
    int64_t m_send_time_us;
    int64_t m_recv_time_ms;
    int64_t m_recv_time_us;
};

} // namespace messages
} // namespace util
} // namespace ipc