/*
 * @file socketwrapper.h
 * @author Songxi Yang
 * @mail ysx-cpp@gmail.com
 * @github https://github.com/ysx-cpp
 * @date Oct 08 2019
 */
#ifndef NET_TCP_SOCKET_SETINTS_HPP
#define NET_TCP_SOCKET_SETINTS_HPP

#include <iostream>
#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/noncopyable.hpp>

namespace fastlink {
namespace ipc {

using tcpsocket = boost::asio::ip::tcp::socket;

class TcpSocketSettings : 
    private boost::noncopyable,
    public boost::enable_shared_from_this<TcpSocketSettings>
{
public:
    explicit TcpSocketSettings(tcpsocket &&socket)
        : socket_(std::move(socket))
    {
    }

    virtual ~TcpSocketSettings()
    {
    }

    void SetSocketReuse(bool reuse)
    {
        tcpsocket::reuse_address ra(reuse);
        socket_.set_option(ra);
    }

    void SetSendBuffSize(size_t buff_size)
    {
        tcpsocket::send_buffer_size sbs(static_cast<int>(buff_size));
        socket_.set_option(sbs);
    }

    size_t GetSendBuffSize()
    {
        tcpsocket::send_buffer_size sbs;
        socket_.get_option(sbs);

        std::cout << "send_buffer_size= " << sbs.value() << std::endl;

        return sbs.value();
    }

    void SetReciveBuffSize(size_t buff_size)
    {
        tcpsocket::receive_buffer_size rbs(static_cast<int>(buff_size));
        socket_.set_option(rbs);
    }

    size_t GetReciveBuffSize()
    {
        tcpsocket::receive_buffer_size rbs;
        socket_.get_option(rbs);
    
        std::cout << "send_buffer_size= " << rbs.value() << std::endl;

        return rbs.value();
    }

    std::string LocalAddress() const
    {
        auto endpoint = socket_.local_endpoint();
        return endpoint.address().to_string() + ":" + std::to_string(endpoint.port());
    }

    std::string PeerAddress() const
    {
        auto endpoint = socket_.remote_endpoint();
        return endpoint.address().to_string() + ":" + std::to_string(endpoint.port());
    }

    int socketfd()
    {
        return static_cast<int>(socket_.native_handle());
    }

protected:
    tcpsocket socket_;
};
    
} // namespace ipc
} // namespace fastlink

#endif // NET_TCP_SOCKET_SETINTS_HPP