#include "httpserver.h"
#include <iostream>
#include <string>
#include <boost/asio/spawn.hpp>

namespace ipc {
namespace net {

namespace asio = boost::asio;
namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
using boost::asio::ip::tcp;

Session::Session(boost::asio::io_context &ioc, HttpServer *server) :
    server_(server), stream_(ioc)
{
}

void Session::DoRead()
{
    req_ = {};

    // Set the timeout.
    stream_.expires_after(std::chrono::seconds(30));

    // Read a request
    http::async_read(stream_, buffer_, req_,
                     beast::bind_front_handler(
                         &Session::OnRead,
                         shared_from_this()));
}

void Session::OnRead(const boost::system::error_code &ec, const std::size_t &bytes_transferred)
{
    boost::ignore_unused(bytes_transferred);

    // This means they closed the session
    if (ec == http::error::end_of_stream)
        return Close();

    if (ec)
    {
        NET_LOGERR("error message: " << ec.message() << "\n");
        return;
    }

    // boost::asio::spawn(server_->io_context(), [this](boost::asio::yield_context yield) mutable {
        if (server_)
            server_->HandleRequest(req_, shared_from_this());
    // });
}

void Session::OnWrite(bool keep_alive, boost::system::error_code ec, std::size_t bytes_transferred)
{
	boost::ignore_unused(bytes_transferred);

    if (ec)
    {
        NET_LOGERR("Exception in handling request: " << ec.message() << "\n");
        return;
    }

    if (!keep_alive)
	{
		// This means we should close the connection, usually because
		// the response indicated the "Connection: close" semantic.
		return Close();
	}

	// We're done with the response so delete it
    res_ = nullptr;

	// Read another request
	DoRead();
}

void Session::Close()
{
    // Send a TCP shutdown
	beast::error_code ec;
	stream_.socket().shutdown(tcp::socket::shutdown_send, ec);

	// At this point the connection is closed gracefully
}

//////////////////////////////////////////////////////////////
//class HttpServer
HttpServer::HttpServer(boost::asio::io_context &ioc, const std::string &host, unsigned short port) : 
    io_context_(ioc), acceptor_(ioc, asio::ip::tcp::endpoint(asio::ip::address::from_string(host), port))
    // acceptor_(app_.io_context(), tcp::endpoint(tcp::v4(), port))
{
}

HttpServer::~HttpServer()
{
}

void HttpServer::Start()
{
    AcceptConnection();
}

void HttpServer::Stop()
{
    acceptor_.cancel();
    acceptor_.close();
    //RemoveAllConnection();
}

void HttpServer::AcceptConnection()
{
    boost::system::error_code ec;
    auto session = std::make_shared<Session>(io_context_, this);
    acceptor_.async_accept(session->socket(), boost::bind(&HttpServer::OnAcceptConnection, this, session, boost::placeholders::_1));
}

void HttpServer::OnAcceptConnection(SessionPtr session,  const boost::system::error_code &ec)
{
    int fd = session->socket().native_handle();
    NET_LOGERR("ACCEPT SUCC error_code:" << ec << " fd:" << fd);
    if (ec)
    {
        std::cout << "delete session" << std::endl;    
        return session->Close();
    }
    AcceptConnection();

    //Start recive
    session->DoRead();
}

// int HttpServer::OnReceveData(const PackagePtr package, ConnectionPtr session)
// {
//     // boost::asio::spawn(io_context(), [this, session, package](boost::asio::yield_context yield) mutable {
//         // 创建一个空的buffer
//         beast::flat_buffer buffer;

//         // 将原始数据放入buffer
//         asio::buffer_copy(buffer.prepare(package->data().size()), boost::asio::buffer(package->data()));
//         buffer.commit(package->data().size());

//         // 创建一个HTTP请求对象
//         http::request<http::string_body> req;

//         // 创建HTTP解析器
//         http::request_parser<http::string_body> parser(req);

//         // 解析buffer中的数据
//         beast::error_code ec;
//         parser.put(buffer.data(), ec);

//         std::string debugstring(package->data().begin(), package->data().end());
//         NET_LOGERR("debugstring: " << debugstring << "\n");

//         try
//         {
//             if (ec) NET_LOGERR("parser.put: " << ec.message() << "\n");

//             HandleRequest(req, session);
//         }
//         catch (std::exception &e)
//         {
//             NET_LOGERR("Exception in handling request: " << e.what() << "\n");
//         }
//     // });

//     return 0;
// }

void HttpServer::HandleRequest(const boost::beast::http::request<boost::beast::http::string_body> &req, SessionPtr session)
{
    try
    {
        boost::system::error_code ec;
        if (!ec)
        {
            // std::istream request_stream(&request);
            // std::string method, path, http_version;
            // request_stream >> method >> path >> http_version;

            std::string response =
                "HTTP/1.1 200 OK\r\n"
                "Content-Type: text/plain\r\n"
                "Content-Length: 13\r\n"
                "Connection: close\r\n\r\n"
                "Hello, world!";

            // boost::asio::async_write(session->socket(), boost::asio::buffer(response), yield[ec]);
            boost::asio::async_write(session->socket(), boost::asio::buffer(response), [session](const boost::system::error_code &ec, const std::size_t &write_bytes) {
                NET_LOGERR("Response messge success wirte bytes:" << write_bytes);
                if (session) session->Close();
            });
        }
    }
    catch (std::exception &e)
    {
        NET_LOGERR("Exception in handling request: " << e.what() << "\n");
    }
}

} // namespace net
} // namespace ipc

