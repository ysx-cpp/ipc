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

namespace fastlink {
namespace ipc {

class MatchWhitespace
{
public:
    explicit MatchWhitespace(char c) : c_(c) {}

    template <typename Iterator>
    std::pair<Iterator, bool> operator()(
        Iterator begin, Iterator end) const
    {
        Iterator i = begin;
        while (i != end)
            if (c_ == *i++)
                return std::make_pair(i, true);
        return std::make_pair(i, false);
    }

private:
  char c_;
};

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


    void ReadUntil(MatchWhitespace &match_whitespace);
    void ReadUntilHandler(const boost::system::error_code &ec, const std::size_t &read_bytes);

    virtual void Complete(const ByteArrayPtr data) = 0;
    virtual void Disconnect() = 0;

private:
    boost::asio::streambuf recv_buff_;
	boost::asio::streambuf send_buff_;
};

} // namespace ipc
} // namespace fastlink

#endif // NET_TCP_HANDLER_H
