#include "httpserver.h"
#include <iostream>
#include <string>
#include <boost/asio/spawn.hpp>

namespace ipc {
namespace net {

using boost::asio::ip::tcp;
using boost::asio::ip::address;

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
    auto connection = CreateConnection(app_.io_context(), this);
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
    // boost::asio::spawn(io_context(), [this, connection, package](boost::asio::yield_context yield) mutable {
        std::string msg(package->data().begin(), package->data().end());
        HandleRequest(msg, connection);
    // });

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
            // boost::asio::async_write(connection->socket_, boost::asio::buffer(response), yield[ec]);
        }
    }
    catch (std::exception &e)
    {
        std::cerr << "Exception in handling request: " << e.what() << "\n";
    }
}

} // namespace net
} // namespace ipc

