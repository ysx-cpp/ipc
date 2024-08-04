#include "httpserver.h"
#include <iostream>
#include <string>
#include <boost/asio/spawn.hpp>

#include "connection.h"

namespace ipc {
namespace net {

using boost::asio::ip::tcp;
using boost::asio::ip::address;

class Session : public Connection
{
public:
    explicit Session(boost::asio::io_context &ioc, ConnectionPool *connction_pool) :
    Connection(ioc, connction_pool)
    {
    }

    void Complete(const ByteArrayPtr data) override
    {
        auto package = std::make_shared<Package>();
        package->FullData(*data);

        std::string stringmsg(package->data().begin(), package->data().end());
        NET_LOGINFO("INFO verify1:" << package->verify() << " cmd:" << package->cmd() << " data:" << stringmsg << " size:" << package->data().size());

        if (connction_pool_)
            connction_pool_->OnReceveData(package, ShaerdSelf());
        else
            OnReceveData(package);
    }
};

HttpServer::HttpServer(const std::string &host, unsigned short port) : 
app_(ApplicationSingle::instance()),
acceptor_(app_.io_context(), tcp::endpoint(address::from_string(host), port))
// acceptor_(app_.io_context(), tcp::endpoint(tcp::v4(), port))
{
}

HttpServer::~HttpServer()
{
}

void HttpServer::Start()
{
    AcceptConnection();
    app_.Run();
}

void HttpServer::Stop()
{
    acceptor_.cancel();
    acceptor_.close();
    RemoveAllConnection();
}

void HttpServer::AcceptConnection()
{
    boost::system::error_code ec;
    auto connection = std::make_shared<Session>(app_.io_context(), this);
    acceptor_.async_accept(connection->socket_, boost::bind(&HttpServer::OnAcceptConnection, this, connection, boost::placeholders::_1));
}

void HttpServer::OnAcceptConnection(ConnectionPtr connection,  const boost::system::error_code &ec)
{
    int fd = connection->impl_.GetSocketFD();
    NET_LOGERR("ACCEPT SUCC error_code:" << ec << " fd:" << fd);
    if (ec)
    {
        std::cout << "delete session" << std::endl;    
        return connection->Stop();
    }
    AcceptConnection();

    //Start recive
    connection->Start();
    connection->ReadUntil("\r\n\r\n");
}

int HttpServer::OnReceveData(const PackagePtr package, ConnectionPtr connection)
{
    boost::asio::spawn(io_context(), [this, connection, package](boost::asio::yield_context yield) mutable {
        std::string msg(package->data().begin(), package->data().end());
        HandleRequest(msg, connection);
    });

    return 0;
}

void HttpServer::HandleRequest(const std::string &data, ConnectionPtr connection)
{
    try
    {
        boost::system::error_code ec;
        boost::asio::streambuf request;
        ///boost::asio::async_read_until(connection->socket_, request, "\r\n\r\n");

        if (!ec)
        {
            std::istream request_stream(&request);
            std::string method, path, http_version;
            request_stream >> method >> path >> http_version;

            std::string response =
                "HTTP/1.1 200 OK\r\n"
                "Content-Type: text/plain\r\n"
                "Content-Length: 13\r\n"
                "Connection: close\r\n\r\n"
                "Hello, world!";

            ByteArray msg(response.begin(), response.end());
            connection->Write(msg);
            connection->Stop();

            // boost::asio::async_write(connection->socket_, boost::asio::buffer(response), yield[ec]);
            // boost::asio::async_write(connection->socket_, boost::asio::buffer(response), [connection](const boost::system::error_code &ec, const std::size_t &write_bytes) {
            //     NET_LOGERR("Response messge success wirte bytes:" << write_bytes);
            //     if (connection) connection->Stop();
            // });
        }
    }
    catch (std::exception &e)
    {
        NET_LOGERR("Exception in handling request: " << e.what() << "\n");
    }
}

} // namespace net
} // namespace ipc
