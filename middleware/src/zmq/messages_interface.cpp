#include "messages_interface.h"

namespace ipc {
namespace util {
namespace messages {

//////////////////////////////////////////////////////////////////////////////////
ZmqMessagesClient::ZmqMessagesClient(const std::string& client_id) {
    m_id = client_id;
    m_addr = ipc_SERVER_REQ_ADDR;
    m_port = 7712;
    m_started = false;
    start();
}
ZmqMessagesClient::ZmqMessagesClient(const std::string& client_id, const std::string& server, uint16_t port) {
    m_id = client_id;
    m_addr = server;
    m_port = port;
    m_started = false;
    start();
}

bool ZmqMessagesClient::start() {
    if (m_started == false) {
        m_client_ptr = std::make_shared<ipc::util::messages::MessagesClient>(m_id, m_addr, m_port);
        m_started = true;
    }
    return true;
}

void ZmqMessagesClient::spin() {
    m_client_ptr->spin();
}

void ZmqMessagesClient::stop() {
    m_client_ptr->stop();
}

bool ZmqMessagesClient::connected() {
    return m_client_ptr->connected();
}

bool ZmqMessagesClient::publish(const std::string& topic, const std::string& message) {
    return m_client_ptr->publish<std::string>(topic, message);
}

bool ZmqMessagesClient::publish(const std::string& topic, const std::shared_ptr<std::string>& message) {
    return m_client_ptr->publish<std::string>(topic, *message);
}

bool ZmqMessagesClient::subscribe(const std::string& topic, std::function<void(const std::string&)> callback_func) {
    return m_client_ptr->subscribe<std::string>(topic, callback_func);
}

bool ZmqMessagesClient::register_subscription(const std::string& topic, CallbackInterface callback) {
    return m_client_ptr->register_subscription(topic, callback);
}

//////////////////////////////////////////////////////////////////////////////////
ZmqMessagesServer::ZmqMessagesServer() {
    m_name = "ZMQServer";
    m_port = 7712;
}

ZmqMessagesServer::ZmqMessagesServer(std::string name, uint16_t port) {
    m_name = name;
    m_port = port;
}

bool ZmqMessagesServer::start() {
    m_server_ptr = std::make_shared<ipc::util::messages::RoutingServer>(m_port);
    m_server_ptr->run();
    return true;
}

void ZmqMessagesServer::stop() {
    m_server_ptr->stop();
}

}
}
}