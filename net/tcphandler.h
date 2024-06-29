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
#include "sockethandler.hpp"

namespace ipc {
namespace net {

class TcpHandler : public SocketHandler<boost::asio::ip::tcp::socket>
{
public:
	explicit TcpHandler(boost::asio::io_context &ioc);

    void Write(const ByteArray &data);
    void Read();

protected:
    void WriteSome(const ByteArray &data);
    void WriteSomeHandler(const boost::system::error_code &ec, const std::size_t &write_bytes);
    void ReadSome();
    void ReadSomeHandler(const boost::system::error_code &ec, const std::size_t &read_bytes);


    void ReadUntil(const std::string& string_regex);
    void ReadUntilHandler(const boost::system::error_code &ec, const std::size_t &read_bytes);

    virtual void Complete(const ByteArrayPtr data) = 0;
    virtual void Successfully(const std::size_t &write_bytes) = 0;
    virtual void Shutdown() = 0;

private:
    boost::asio::streambuf recv_buff_;
	boost::asio::streambuf send_buff_;
};

} // namespace net
} // namespace ipc

#endif // NET_TCP_HANDLER_H
