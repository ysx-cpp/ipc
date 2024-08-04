/*
 * @file tcphandler.h
 * @author Songxi Yang
 * @mail ysx-cpp@gmail.com
 * @github https://github.com/ysx-cpp
 * @date Oct 08 2019
 */
#ifndef NET_HTTP_SERVER_H
#define NET_HTTP_SERVER_H

#include <boost/asio.hpp>
#include <boost/beast/http.hpp>
#include "connectionpool.h"
#include "application.h"

namespace ipc {
namespace net {

class HttpServer  : public ConnectionPool
{
public:
    explicit HttpServer(const std::string &host, unsigned short port);
    ~HttpServer() override;
    
    void Start();
    void StartThreadPool();
    void Stop();

    boost::asio::io_context &io_context() 
    { return app_.io_context(); }

    std::shared_ptr<boost::asio::io_context> &io_context_shared() 
    { return app_.io_context_shared(); }

    int OnReceveData(const PackagePtr package, ConnectionPtr connection) override;
    void OnSendData(const std::size_t& write_bytes, ConnectionPtr connection) override {}
	int OnConnect(ConnectionPtr connection) override {return 0;}
	int OnDisconnect(ConnectionPtr connection) override {return 0;}

    virtual void HandleRequest(const boost::beast::http::request<boost::beast::http::string_body> &req, ConnectionPtr connection);
    void HandleResponse(const boost::beast::http::response<boost::beast::http::string_body> &res, ConnectionPtr connection);
    void HandleResponse(const boost::beast::http::response<boost::beast::http::empty_body> &res, ConnectionPtr connection);
    void HandleResponse(const boost::beast::http::response<boost::beast::http::file_body> &res, ConnectionPtr connection);

private:
    void AcceptConnection();
    void OnAcceptConnection(ConnectionPtr connection,  const boost::system::error_code &ec);

private:
    Application &app_;
    boost::asio::ip::tcp::acceptor acceptor_;
};

} // namespace net
} // namespace ipc
#endif // NET_HTTP_SERVER_H