// zeromq package/unpackage & send/recv implement

#include "zmq.hpp"
#include "zmq_envelope.h"

namespace ipc {
namespace util {
namespace messages {

////////////////////////////////////////////////////////////////////////////////
// send & recv

bool send_zmq(std::shared_ptr<zmq::socket_t>& socket, std::mutex& mutex,
              const std::string& buffer) {
    try {
        zmq::message_t message(buffer.c_str(), buffer.length());
        std::lock_guard<std::mutex> lock(mutex);
        return socket->send(buffer.c_str(), buffer.length());
    } catch (zmq::error_t& e) {
        DEBUG_ERROR(e.what());
        return false;
    }
}

bool recv_zmq(std::shared_ptr<zmq::socket_t>& socket, std::mutex& mutex, std::string& buffer) {
    try {
        zmq::message_t message;
        {
            std::lock_guard<std::mutex> lock(mutex);

            if (socket->recv(&message) == false) {
                return false;
            }
        }
        buffer = std::string(static_cast<char*>(message.data()), message.size());
        return true;
    } catch (zmq::error_t& e) {
        DEBUG_ERROR(e.what());
        return false;
    }
}

////////////////////////////////////////////////////////////////////////////////
MessageEnvelope::MessageEnvelope(const std::string& topic, const std::string& publisher,
                                 const std::string& payload, const int64_t serial) {
    this->fill(topic, publisher, payload, serial);
}

void MessageEnvelope::fill(const std::string& topic, const std::string& publisher,
                           const std::string& payload, const int64_t serial) {
    m_topic = topic;
    m_publisher = publisher;
    m_payload = payload;
    m_serial = serial;
}

// send & recv
bool MessageEnvelope::send(zmq::socket_t& zmq_socket) {
    // send time stamp in microseconds
    int64_t _now_ms = now_ms();
    std::string _send_time_ms = std::to_string(_now_ms);
    int64_t _now_us = now_ms();
    std::string _send_time_us = std::to_string(_now_us);
    std::string _serial = std::to_string(m_serial);

    zmq::message_t topic(m_topic.c_str(), m_topic.length());
    zmq::message_t publisher(m_publisher.c_str(), m_publisher.length());
    zmq::message_t payload(m_payload.c_str(), m_payload.length());
    zmq::message_t serial(_serial.c_str(), _serial.length());
    zmq::message_t send_time_ms(_send_time_ms.c_str(), _send_time_ms.length());
    zmq::message_t send_time_us(_send_time_us.c_str(), _send_time_us.length());

    try {
        // must keep same order with this->recv()
        if (zmq_socket.send(topic, ZMQ_SNDMORE) == false) {
            return false;
        }

        if (zmq_socket.send(publisher, ZMQ_SNDMORE) == false) {
            return false;
        }

        if (zmq_socket.send(payload, ZMQ_SNDMORE) == false) {
            return false;
        }

        if (zmq_socket.send(serial, ZMQ_SNDMORE) == false) {
            return false;
        }

        if (zmq_socket.send(send_time_ms, ZMQ_SNDMORE) == false) {
            return false;
        }

        // the last frame
        if (zmq_socket.send(send_time_us) == false) {
            return false;
        }

        return true;
    } catch (zmq::error_t& e) {
        DEBUG_ERROR(e.what());
        return false;
    }
}

bool MessageEnvelope::recv(zmq::socket_t& zmq_socket) {
    // recv time stamp in microseconds

    zmq::message_t topic;
    zmq::message_t publisher;
    zmq::message_t payload;
    zmq::message_t serial;
    zmq::message_t send_time_ms;
    zmq::message_t send_time_us;

    try {
        int32_t recv_more;
        size_t size_more = sizeof(size_more);

        // must keep same order with this->send()
        if (!zmq_socket.recv(&topic)) {
            return false;
        }

        zmq_socket.getsockopt(ZMQ_RCVMORE, &recv_more, &size_more);

        if (!recv_more || !zmq_socket.recv(&publisher)) {
            return false;
        }

        zmq_socket.getsockopt(ZMQ_RCVMORE, &recv_more, &size_more);

        if (!recv_more || !zmq_socket.recv(&payload)) {
            return false;
        }

        zmq_socket.getsockopt(ZMQ_RCVMORE, &recv_more, &size_more);

        if (!recv_more || !zmq_socket.recv(&serial)) {
            return false;
        }

        zmq_socket.getsockopt(ZMQ_RCVMORE, &recv_more, &size_more);

        if (!recv_more || !zmq_socket.recv(&send_time_ms)) {
            return false;
        }

        zmq_socket.getsockopt(ZMQ_RCVMORE, &recv_more, &size_more);

        if (!recv_more || !zmq_socket.recv(&send_time_us)) {
            return false;
        }

        m_topic = std::string(static_cast<char*>(topic.data()), topic.size());
        m_publisher = std::string(static_cast<char*>(publisher.data()), publisher.size());
        m_payload = std::string(static_cast<char*>(payload.data()), payload.size());
        m_serial = safe_stoll(std::string(static_cast<char*>(serial.data()), serial.size()));
        m_send_time_ms = safe_stoll(std::string(static_cast<char*>(send_time_ms.data()),
                                                send_time_ms.size()));
        m_send_time_us = safe_stoll(std::string(static_cast<char*>(send_time_us.data()),
                                                send_time_us.size()));
        m_recv_time_ms = now_ms();
        m_recv_time_us = now_us();

        return true;

    } catch (zmq::error_t& e) {
        DEBUG_ERROR(e.what());
        return false;
    }
}

} // namespace messages
} // namespace util
} // namespace ipc
