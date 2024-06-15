
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
        DoHeartBeat();
        // Package pkg;
        // std::copy(strmsg.begin(), strmsg.end(), pkg.data_.begin());
        Send(strmsg);
    }

    void set_host(const std::string &host) { host_ = host; };
    const std::string &host() const { return host_; }
    void set_port(uint16_t port) { port_ = port; }
    uint16_t port() const { return port_; }

protected:
    void Complete(const ByteArrayPtr data) override
    {
        if (data == nullptr) return;

        std::string msg(data->begin(), data->end());
        std::cout << msg << std::endl;
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
        std::cout << data->pdata() << " seq:" << data->seq() << std::endl;

        Package pkg;
        pkg.Encode("Hello client!");
        connection->Write(const_cast<ByteArray&>(pkg.data()));
        return 0;
    }
	void OnSendData(const std::size_t& write_bytes) override
    {
    }
	int OnConnect(ConnectionPtr connection) override
    {
        return 0;
    }
	int OnDisconnect(ConnectionPtr connection) override
    {
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