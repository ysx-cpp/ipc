
#include <string.h>
#include <iostream>
#include <string>
#include <functional>
#include "../tcpclient.h"
#include "../tcpserver.h"

using ipc::net::TcpClient;
using ipc::net::TcpServer;
using ipc::net::PackagePtr;
using ipc::net::ConnectionPtr;
using ipc::net::Package;
using ipc::net::ByteArrayPtr;
using ipc::net::ByteArray;

class TestTcpClient : public TcpClient
{
public:
	explicit TestTcpClient(boost::asio::io_context &ioc): 
    TcpClient(ioc)
    {
    }

    void TestPing()
    {
        std::string strmsg("Hello server!");
        std::cout << strmsg << std::endl;

        PackagePtr pkg = std::make_shared<Package>();
        pkg->set_cmd(10000);
        SendPackage(pkg, strmsg);

        SendHeartbeat();
    }

    void set_host(const std::string &host) { host_ = host; };
    const std::string &host() const { return host_; }
    void set_port(uint16_t port) { port_ = port; }
    uint16_t port() const { return port_; }

protected:
    int OnReceveData(const PackagePtr package) override
    {
        if (package == nullptr) 
            return 0;

        std::string msg(package->data().begin(), package->data().end());
        std::cout << msg << std::endl;
        return 0;
    }
    void OnSendData(const std::size_t& write_bytes) override
    {
        std::cout << __FUNCTION__ << "|OnSendData write_bytes:" << write_bytes << std::endl;
    }
    int OnConnect() override
    {
        std::cout << __FUNCTION__ << "|OnConnect"<< std::endl;
        return 0;
    }
	int OnDisconnect() override
    {
        std::cout << __FUNCTION__ << "|OnDisconnect" << std::endl;
        return 0;
    }
private:
	std::string host_;
	uint16_t port_;
};


class TestTcpServer : public TcpServer
{
public:
    TestTcpServer(const std::string &host, unsigned short port) : TcpServer(host, port)
    {
    }

    int OnReceveData(const PackagePtr data, ConnectionPtr connection) override
    {
        std::cout << __FUNCTION__ << " |data:" << data->pdata() << "| seq:" << data->seq() << std::endl;

        Package pkg;
        pkg.set_cmd(0);
        if (connection)
            connection->SendData(pkg, "Hello client!");
        return 0;
    }
	void OnSendData(const std::size_t& write_bytes, ConnectionPtr connection) override
    {
        std::cout << __FUNCTION__ << "|OnSendData write_bytes:" << write_bytes << std::endl;
    }
	int OnConnect(ConnectionPtr connection) override
    {
        std::cout << __FUNCTION__ << "|OnConnect:" << connection << std::endl;
        return 0;
    }
	int OnDisconnect(ConnectionPtr connection) override
    {
        std::cout << __FUNCTION__ << "|OnDisconnect"  << std::endl;
        return 0;
    }
};

int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        std::cerr << "param error!" << std::endl;
        return __LINE__;
    }

    std::string host = argv[2];
    unsigned short port = std::stod(argv[3]);

    if (strcmp(argv[1], "client") == 0)
    {
        boost::asio::io_context ioc;
        TestTcpClient client(ioc);
        client.CreateConnect(host, port);
        // client.Connect(host, port);
        client.TestPing();
        ioc.run();
    }
    else if (strcmp(argv[1], "server") == 0)
    {
        TestTcpServer server(host, port);
        server.Start();
    }
    else
    {
        std::cerr << "Param error! please input client or server." << std::endl;
        return __LINE__;
    }
        
    return 0;
}