/*
 * @file udphandler.h
 * @author Songxi Yang
 * @mail ysx-cpp@gmail.com
 * @github https://github.com/ysx-cpp
 * @date Oct 08 2019
 */
#ifndef NET_TCP_HANDLER_H
#define NET_TCP_HANDLER_H

#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>
#include <boost/asio/streambuf.hpp>
#include "package.h"
#include "sockethandler.hpp"

namespace ipc {
namespace net {

class UdpHandler : public SocketHandler<boost::asio::ip::udp::socket>
{
public:
    using UdpEndpoint = boost::asio::ip::udp::endpoint;

    UdpHandler(boost::asio::io_context &ioc, UdpEndpoint &&endpoint);

protected:
    void SendTo(const ByteArray &data, UdpEndpoint &endpoint);
	void SendHandler(const boost::system::error_code &ec, const std::size_t &write_bytes);
	void Receive();
    void ReceiveFrom(UdpEndpoint &endpoint);
	void ReceiveHandler(const boost::system::error_code &ec, const std::size_t &read_bytes);
    void OnReceive(const std::size_t &read_bytes, const UdpEndpoint &endpoint);
    virtual void Complete(const ByteArrayPtr data, const UdpEndpoint &endpoint);
	virtual void Disconnect();

private:
	boost::asio::streambuf recv_buff_;
	boost::asio::streambuf send_buff_;
};

} // namespace net
} // namespace ipc

#endif // NET_TCP_HANDLER_H
