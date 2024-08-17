/*
 * @file tcphandler.h
 * @author Songxi Yang
 * @mail ysx-cpp@gmail.com
 * @github https://github.com/ysx-cpp
 * @date Oct 08 2019
 */
#ifndef NET_TCP_HANDLER_H
#define NET_TCP_HANDLER_H

#include <boost/system/error_code.hpp>
#include <boost/asio/streambuf.hpp>
#include "package.h"
// #include "sockethandler.hpp"
#include "socketsettings.hpp"

namespace ipc {
namespace net {

class TcpHandler : public std::enable_shared_from_this<TcpHandler>
{
    using TcpSocket = boost::asio::ip::tcp::socket;
    using TcpEndpoint = boost::asio::ip::tcp::endpoint;
public:
	explicit TcpHandler(boost::asio::io_context &ioc);
    virtual ~TcpHandler() = default;

    void Write(const ByteArray &data);
    void Read();

protected:
    void WriteSome(const ByteArray &data);
    void WriteSomeHandler(const boost::system::error_code &ec, const std::size_t &write_bytes);
    void ReadSome();
    void ReadSomeHandler(const boost::system::error_code &ec, const std::size_t &read_bytes);

    void ReadUntil(const std::string& string_regex);
    void ReadUntilHandler(const std::string& string_regex, const boost::system::error_code &ec, const std::size_t &read_bytes);
    
    virtual void Complete(const std::string &data) = 0;
    virtual void Successfully(const std::size_t &write_bytes) = 0;
    virtual void Shutdown() = 0;

protected:
    friend class TcpServer;
    friend class HttpServer;
    TcpSocket socket_;
    SocketSettings<TcpSocket> impl_;

private:
    boost::asio::streambuf recv_buff_;
	boost::asio::streambuf send_buff_;
};

} // namespace net
} // namespace ipc

#endif // NET_TCP_HANDLER_H
