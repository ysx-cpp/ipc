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

#include <boost/beast/core.hpp>
#include <boost/beast/version.hpp>

namespace ipc {
namespace net {

class HttpServer; 
class Session : public std::enable_shared_from_this<Session>
{
public:
    explicit Session(boost::asio::io_context &ioc, HttpServer *server);

    void Close();
    void DoRead();
    boost::asio::ip::tcp::socket &socket() {return stream_.socket();}

    template <bool isRequest, class Body, class Fields>
    void DoWirte(boost::beast::http::message<isRequest, Body, Fields> &&rsp)
    {
        // The lifetime of the message has to extend
		// for the duration of the async operation so
		// we use a shared_ptr to manage it.
		auto sp = std::make_shared< boost::beast::http::message<isRequest, Body, Fields> >(std::move(rsp));

		// Store a type-erased version of the shared
		// pointer in the class to keep it alive.
		res_ = sp;

		// Write the response
		boost::beast::http::async_write(stream_, *sp,
			boost::beast::bind_front_handler(
				&Session::OnWrite,
				shared_from_this(),
				sp->need_eof()));
    }

private:
    void OnRead(const boost::system::error_code &ec, const std::size_t &bytes_transferred);
    void OnWrite(bool keep_alive, boost::system::error_code ec, std::size_t bytes_transferred);

private:
    HttpServer *server_;
    boost::beast::tcp_stream stream_;
	boost::beast::flat_buffer buffer_;
	boost::beast::http::request<boost::beast::http::string_body> req_;
    std::shared_ptr<void> res_;
};
using SessionPtr = std::shared_ptr<Session>;

//////////////////////////////////////////////////////////////
//class HttpServer
class HttpServer : private boost::noncopyable
{
public:
    HttpServer(boost::asio::io_context &ioc, const std::string &host, unsigned short port);
    ~HttpServer();
    
    void Start();
    void Stop();

    boost::asio::io_context &io_context() { return io_context_; }
    virtual void HandleRequest(const boost::beast::http::request<boost::beast::http::string_body> &req, SessionPtr session);

private:
    void AcceptConnection();
    void OnAcceptConnection(SessionPtr session,  const boost::system::error_code &ec);

private:
    boost::asio::io_context &io_context_;
    boost::asio::ip::tcp::acceptor acceptor_;
};

} // namespace net
} // namespace ipc
#endif // NET_HTTP_SERVER_H