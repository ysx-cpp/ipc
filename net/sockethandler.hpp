/*
 * @file socketwrapper.h
 * @author Songxi Yang
 * @mail ysx-cpp@gmail.com
 * @github https://github.com/ysx-cpp
 * @date Oct 08 2019
 */
#ifndef NET_SOCKET_WRAPPER_HPP
#define NET_SOCKET_WRAPPER_HPP

#include <iostream>
#include <memory>
#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>
#include <boost/noncopyable.hpp>


namespace fastlink {
namespace ipc {

template<class T>
class SocketHandler :
	private boost::noncopyable,
	public std::enable_shared_from_this<SocketHandler<T>>
{
public:
    explicit SocketHandler(boost::asio::io_context &ioc)
        : socket_(ioc)
    {
    }

    template <typename ContextType, typename Tp>
    SocketHandler(ContextType &ioc, boost::asio::ip::basic_endpoint<Tp> &&ep)
        : socket_(ioc, std::move(ep))
    {
    }

    virtual ~SocketHandler()
    {
        Close();
    }

    void SetSocketReuse(bool reuse)
    {
        typename T::reuse_address option(reuse);
        socket_.set_option(option);
    }

    void SetSendBuffSize(size_t buff_size)
    {
        // typename T::send_buffer_size option(static_cast<int>(buff_size));
        boost::asio::socket_base::receive_buffer_size option(static_cast<int>(buff_size));
        socket_.set_option(option);
    }

    template <class Tp>
    size_t GetSendBuffSize(Tp &&protocol)
    {
        typename T::send_buffer_size option;
        socket_.get_option(option);

        return option.size(protocol);
    }
    
    size_t GetSendBuffSize()
    {
        typename T::send_buffer_size option;
        socket_.get_option(option);

        return option.value();
    }

    void SetReciveBuffSize(size_t buff_size)
    {
        typename T::receive_buffer_size option(static_cast<int>(buff_size));
        socket_.set_option(option);
    }

    template <class Tp>
    size_t GetReciveBuffSize(Tp &&protocol)
    {
        typename T::receive_buffer_size option;
        socket_.get_option(option);

        return option.size(protocol);
    }

    size_t GetReciveBuffSize()
    {
        typename T::receive_buffer_size rbs;
        socket_.get_option(rbs);

        return rbs.value();
    }

    std::string LocalAddress() const
    {
        auto &endpoint = socket_.local_endpoint();
        return endpoint.address().to_string() + ":" + std::to_string(endpoint.port());
    }

    std::string PeerAddress() const
    {
        auto &endpoint = socket_.remote_endpoint();
        return endpoint.address().to_string() + ":" + std::to_string(endpoint.port());
    }

    int socketfd()
    {
        return static_cast<int>(socket_.native_handle());
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

    void Close()
    {
        std::cout << "close fd:" << socketfd() << std::endl;
        if (socket_.is_open())
        {
            boost::system::error_code ec;
            socket_.close(ec);
            if (!ec)
                socket_.shutdown(T::shutdown_both, ec);
        }
    }

protected:
    friend class TcpServer;
    friend class UdpServer;
    friend class IcmpServer;
    T socket_;
};

} // namespace ipc
} // namespace fastlink

#endif // NET_SOCKET_WRAPPER_HPP
