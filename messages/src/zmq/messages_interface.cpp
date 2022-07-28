#include "messages_interface.h"

namespace ipc {
namespace messages {
    
//////////////////////////////////////////////////////////////////////////////////
class ZmqMessagesServer : public IServerNode
{
public:
    ZmqMessagesServer() :
    m_name("ZMQServer"),
    m_port(7712)
    {
    }

    ZmqMessagesServer(std::string name, uint16_t port) :
    m_name("name"),
    m_port(port)
    {
    }

    bool start() override
    {
        m_server_ptr = std::make_shared<ipc::messages::RoutingServer>(m_port);
        m_server_ptr->run();
        return true;
    }

    void spin() override
    {
    }

    void stop() override
    {
        m_server_ptr->stop();
    }

private:
    std::string m_name;
    uint16_t m_port;
    std::shared_ptr<ipc::messages::RoutingServer> m_server_ptr;
};

IServerNode* CreateServerNode(const std::string& name, uint16_t port)
{
    return new ZmqMessagesServer(name, port);
}

//////////////////////////////////////////////////////////////////////////////////
class ZmqMessagesClient : public IClientNode
{
public:
    ZmqMessagesClient(const std::string &client_id) :
    m_started(false),
    m_id(client_id),
    m_addr(ipc_SERVER_REQ_ADDR),
    m_port(7712)
    {
        start();
    }
    ZmqMessagesClient(const std::string &client_id, const std::string &server, uint16_t port) :
    m_started(false),
    m_id(client_id),
    m_addr(server),
    m_port(port)
    {
        start();
    }

    bool start() override
    {
        if (m_started == false)
        {
            m_client_ptr = std::make_shared<ipc::messages::MessagesClient>(m_id, m_addr, m_port);
            m_started = true;
        }
        return true;
    }

    void spin() override
    {
        m_client_ptr->spin();
    }

    void stop() override
    {
        m_client_ptr->stop();
    }

    bool connected() override
    {
        return m_client_ptr->connected();
    }

    bool publish(const std::string &topic, const std::string &message) override
    {
        return m_client_ptr->publish<std::string>(topic, message);
    }

    bool publish(const std::string &topic, const std::shared_ptr<std::string> &message) override
    {
        return m_client_ptr->publish<std::string>(topic, *message);
    }

    bool subscribe(const std::string &topic, std::function<void(const std::string &)> callback_func) override
    {
        return m_client_ptr->subscribe<std::string>(topic, callback_func);
    }

    bool register_subscription(const std::string &topic, CallbackInterface callback) override
    {
        return m_client_ptr->register_subscription(topic, callback);
    }

private:
    std::atomic<bool> m_started;
    std::string m_id;
    std::string m_addr;
    uint16_t m_port;
    std::shared_ptr<ipc::messages::MessagesClient> m_client_ptr;
};

IClientNode* CreateClientNode(const std::string& name,
    const std::string& ip, uint16_t port)
{
    return new ZmqMessagesClient(name, ip, port);
}

}
}