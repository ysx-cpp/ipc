/*
 * @file udphandler.cpp
 * @author Songxi Yang
 * @mail ysx-cpp@gmail.com
 * @github https://github.com/ysx-cpp
 * @date Oct 08 2019
 */
#include "udphandler.h"
#include <iostream>
#include <assert.h>
#include <boost/system/error_code.hpp>
#include <boost/make_shared.hpp>
#include <boost/bind.hpp>


namespace fastlink {
namespace ipc {

using namespace boost::asio;

UdpHandler::UdpHandler(io_context &ioc, UdpEndpoint &&endpoint)
    : SocketHandler(ioc, std::move(endpoint))
{
}

void UdpHandler::SendTo(const ByteArray & data, UdpEndpoint &endpoint)
{
	if (data.size() > send_buff_.max_size())
		return;

	boost::asio::mutable_buffer buffer = send_buff_.prepare(data.size());
	std::copy(data.cbegin(), data.cend(), static_cast<unsigned char*>(buffer.data()));

	socket_.async_send_to(buffer, endpoint,
		boost::bind(&UdpHandler::SendHandler, this,
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred));
}

void UdpHandler::SendHandler(const boost::system::error_code & ec, const std::size_t & write_bytes)
{
	if (ec || !write_bytes)
	{
		std::cerr << "ERROR write_bytes:" << write_bytes << std::endl;
		return Disconnect();
	}
	send_buff_.consume(write_bytes);
}

void UdpHandler::Receive()
{
	socket_.async_receive(recv_buff_.prepare(GetReciveBuffSize(ip::udp::v4())),
		[this](const boost::system::error_code & ec, const std::size_t & read_bytes) {

		if (!ec && read_bytes)
		{
			boost::system::error_code e;
			OnReceive(read_bytes, socket_.remote_endpoint(e));
			return Receive();
		}
		ReceiveHandler(ec, read_bytes);
	});
}

void UdpHandler::ReceiveFrom(UdpEndpoint &endpoint)
{
	socket_.async_receive_from(recv_buff_.prepare(GetReciveBuffSize(ip::udp::v4())), endpoint,
		[this, &endpoint](const boost::system::error_code & ec, const std::size_t & read_bytes) {

		if (!ec && read_bytes)
		{
			OnReceive(read_bytes, endpoint);
			return ReceiveFrom(endpoint);
		}
		ReceiveHandler(ec, read_bytes);
	});
}

void UdpHandler::ReceiveHandler(const boost::system::error_code & ec, const std::size_t & /*read_bytes*/)
{
	try
	{
		CheckErrorCode(ec);
	}
	catch (const boost::system::system_error &e)
	{
		std::cerr << "error_code:" << e.code().value() << " " << e.what() << std::endl;
		return Disconnect();
	}
}

void UdpHandler::OnReceive(const std::size_t & read_bytes, const UdpEndpoint &endpoint)
{
	recv_buff_.commit(read_bytes);
	boost::asio::streambuf::const_buffers_type buff = recv_buff_.data();
	ByteArrayPtr data = std::make_shared<ByteArray>();
    data->assign(boost::asio::buffers_begin(buff), boost::asio::buffers_begin(buff) + static_cast<int>(read_bytes));
	recv_buff_.consume(read_bytes);
	Complete(data, endpoint);
}

void UdpHandler::Complete(const ByteArrayPtr /*data*/, const UdpEndpoint &/*endpoint*/)
{
}

void UdpHandler::Disconnect()
{
}

} // namespace ipc
} // namespace fastlink
