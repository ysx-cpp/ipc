
#include <string.h>
#include <iostream>
#include <string>
#include <functional>
#include "../tcpclient.h"
#include "../tcpserver.h"
#include "../rpcclient.h"
#include "../logdefine.h"

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
        try
        {
            std::string strmsg("Hello server!");
            NET_LOGINFO(strmsg);

            PackagePtr pkg = std::make_shared<Package>();
            pkg->set_cmd(10000);
            SendPackage(pkg, strmsg);
        }
        catch (const boost::system::system_error &e)
        {
            NET_LOGERR(e.what());
        }
        catch (const std::exception &e)
        {
            NET_LOGERR(e.what());
        }
        catch (...)
        {
            NET_LOGERR("Unknown error");
        }
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
        NET_LOGERR("OnSendData write_bytes:" << write_bytes);
    }
    int OnConnect() override
    {
        NET_LOGERR("OnConnect");
        return 0;
    }
	int OnDisconnect() override
    {
        NET_LOGERR("OnDisconnect");
        return 0;
    }
private:
	std::string host_;
	uint16_t port_;
};


class TestTcpServer : public TcpServer
{
public:
    TestTcpServer(boost::asio::io_context &ioc, const std::string &host, unsigned short port) : 
        TcpServer(ioc, host, port)
    {
    }

    int OnReceveData(const PackagePtr data, ConnectionPtr connection) override
    {
        NET_LOGERR("data:" << data->pdata() << "| seq:" << data->seq());

        Package pkg;
        pkg.set_cmd(1001);
        if (connection)
            connection->SendData(pkg, "Hello client!");
        return 0;
    }
	void OnSendData(const std::size_t& write_bytes, ConnectionPtr connection) override
    {
        NET_LOGERR("OnSendData write_bytes:" << write_bytes)
    }
	int OnConnect(ConnectionPtr connection) override
    {
        NET_LOGERR("OnConnect:" << connection);
        return 0;
    }
	int OnDisconnect(ConnectionPtr connection) override
    {
        if (connection)
            connection->Stop();
        NET_LOGERR("OnDisconnect");
        return 0;
    }
};

int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        NET_LOGERR("param error!");
        return __LINE__;
    }

    std::string host = argv[2];
    unsigned short port = std::stod(argv[3]);
    boost::asio::io_context ioc;

    std::cout << "Put in 'Enter' continue:";
    std::string in;
    std::cin >> in;

    if (strcmp(argv[1], "client") == 0)
    {
        auto client = std::make_shared<TestTcpClient>(ioc);
        client->CreateConnect(host, port);
        client->Connect(host, port);

        client->TestPing();
        client->Start();
        ioc.run();
    }
    else if (strcmp(argv[1], "server") == 0)
    {
        TestTcpServer server(ioc, host, port);
        server.Start();
        ioc.run();
    }
    else if (strcmp(argv[1], "rpc") == 0)
    {
        auto client = std::make_shared<ipc::net::RpcClient>(ioc);
        client->CreateConnect(host, port);
        client->Connect(host, port);

        using ipc::net::RpcContext;
        RpcContext ctx;
        ctx.cmd = 1001;
        ctx.req = "RPC request";
        ctx.call = [](const std::string &rsp) {
            NET_LOGINFO("rsp:" << rsp);
            return 0;
        };
        client->AsyncRequest(ctx);

        client->Start();
        ioc.run();
    }
    else
    {
        NET_LOGERR("Param error! please input client or server." );
        return __LINE__;
    }
        
    return 0;
}