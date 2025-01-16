/*
 * @file socketsettings.h
 * @author Songxi Yang
 * @mail ysx-cpp@gmail.com
 * @github https://github.com/ysx-cpp
 * @date Oct 08 2019
 */
#ifndef NET_SOCKET_SETTINGS_HPP
#define NET_SOCKET_SETTINGS_HPP

#include <iostream>
#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/noncopyable.hpp>
#include "logdefine.h"

namespace ipc {
namespace net {

template<typename T>
class SocketSettings : private boost::noncopyable
{
public:
    explicit SocketSettings(T &socket)
        : socket_(socket)
    {
    }

    virtual ~SocketSettings()
    {
        Close();
    }

    void SetSocketReuse(bool reuse)
    {
        typename T::reuse_address ra(reuse);
        socket_.set_option(ra);
    }

    void SetSendBuffSize(size_t buff_size)
    {
        typename T::send_buffer_size sbs(static_cast<int>(buff_size));
        socket_.set_option(sbs);
    }

    size_t GetSendBuffSize()
    {
        typename T::send_buffer_size sbs;
        socket_.get_option(sbs);

        NET_LOGINFO("send_buffer_size= " << sbs.value());

        return sbs.value();
    }

    void SetReciveBuffSize(size_t buff_size)
    {
        typename T::receive_buffer_size rbs(static_cast<int>(buff_size));
        socket_.set_option(rbs);
    }

    size_t GetReciveBuffSize()
    {
        typename T::receive_buffer_size rbs;
        socket_.get_option(rbs);
    
        NET_LOGINFO("recive_buffer_size= " << rbs.value());

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

    int GetSocketFD()
    {
        return static_cast<int>(socket_.native_handle());
    }

    bool Connected() const 
    {
        return socket_.is_open();
    }
    
    void Close()
    {
        NET_LOGERR("close fd:" << GetSocketFD());
        if (socket_.is_open())
        {
            boost::system::error_code ec;
            socket_.close(ec);
            if (!ec)
                socket_.shutdown(T::shutdown_both, ec);
        }
    }

    int CheckErrorCode(const boost::system::error_code &ec)
    {
        switch (ec.value())
        {
        case boost::system::errc::no_such_file_or_directory:
        {
            boost::asio::detail::throw_error(ec, "no such file or directory");
            break;
        }
        case boost::system::errc::bad_file_descriptor:
        {
            boost::asio::detail::throw_error(ec, "bad file descriptor");
            break;
        }
        case boost::asio::error::operation_aborted:
        {
            boost::asio::detail::throw_error(ec, "operation aborted");
            break;
        }
        default:
            boost::asio::detail::throw_error(ec, "other error");
            break;
        }

        return ec.value();
    }

private:
    T &socket_;
};
    
} // namespace net
} // namespace ipc

#endif // NET_SOCKET_SETTINGS_HPP