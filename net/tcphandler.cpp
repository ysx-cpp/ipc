/*
 * @file tcphandler.cpp
 * @author Songxi Yang
 * @mail ysx-cpp@gmail.com
 * @github https://github.com/ysx-cpp
 * @date Oct 08 2019
 */
#include "tcphandler.h"
#include <iostream>
#include <boost/bind.hpp>
#include "heartbeat.h"

namespace fastlink {
namespace ipc {

using namespace boost::asio;

TcpHandler::TcpHandler(boost::asio::io_context &ioc)
	: SocketHandler(ioc)
{
}

void TcpHandler::Write(const ByteArray &data)
{
	WriteSome(data);
}

void TcpHandler::Read()
{
	ReadSome();
}

void TcpHandler::WriteSome(const ByteArray &data)
{
	if (data.size() > send_buff_.max_size())
		return;

	boost::asio::mutable_buffer buffer = send_buff_.prepare(data.size());
	std::copy(data.cbegin(), data.cend(), static_cast<unsigned char *>(buffer.data()));

	socket_.async_write_some(buffer,
							 boost::bind(&TcpHandler::WriteSomeHandler, this,
										 boost::asio::placeholders::error,
										 boost::asio::placeholders::bytes_transferred));

	// async_write(socket_, boost::asio::buffer(data, data.size()), 
	// [](const boost::system::error_code &ec, const std::size_t &write_bytes) {
	// 	std::cout << "error_code:"<< ec.value() << " write_bytes:" << write_bytes << std::endl;
	// });
}

void TcpHandler::WriteSomeHandler(const boost::system::error_code &ec, const std::size_t &write_bytes)
{
	if (ec || !write_bytes)
	{
		std::cerr << "ERROR write_bytes:" << write_bytes << std::endl;
		return Disconnect();
	}

	send_buff_.consume(write_bytes);

	if (send_buff_.size() > 0)
	{
		socket_.async_write_some(send_buff_.prepare(send_buff_.size()),
								 boost::bind(&TcpHandler::WriteSomeHandler, this,
											 boost::asio::placeholders::error,
											 boost::asio::placeholders::bytes_transferred));
	}
}

void TcpHandler::ReadSome()
{
	socket_.async_read_some(recv_buff_.prepare(GetReciveBuffSize()),
							boost::bind(&TcpHandler::ReadSomeHandler, this,
										boost::asio::placeholders::error,
										boost::asio::placeholders::bytes_transferred));

	/*
    *  boost::asio::streambuf recv_buf;
    *  async_read(*socket_,
    *             recv_buf,
    *             boost::bind(&TcpHandler::ReadHandler, this, _1, _2));
    */
}

void TcpHandler::ReadSomeHandler(const boost::system::error_code &ec, const std::size_t &read_bytes)
{
	try
	{
		CheckErrorCode(ec);
		if (!ec && read_bytes)
		{
			recv_buff_.commit(read_bytes);
			boost::asio::streambuf::const_buffers_type buff = recv_buff_.data();
			const Head *phead = reinterpret_cast<const Head *>(buff.data());
			uint32_t packet_size = phead->head_size + phead->data_size;
			if (phead->head_size > 0)
			{
				if (recv_buff_.size() >= packet_size)
				{
					ByteArrayPtr data = std::make_shared<ByteArray>();
					data->assign(boost::asio::buffers_begin(buff), boost::asio::buffers_begin(buff) + packet_size);
					Complete(data);
					recv_buff_.consume(packet_size);
				}
			}
		}
		ReadSome();
	}
	catch (const boost::system::system_error &e)
	{
		std::cerr << e.what() << std::endl;
		Disconnect();
	}
}

void TcpHandler::ReadUntil(MatchWhitespace &/*match_whitespace*/)
{
	boost::asio::streambuf recv_buf;
	boost::asio::async_read_until(socket_,
								  recv_buf,
								  "q",//MatchWhitespace('a'),
								  boost::bind(&TcpHandler::ReadUntilHandler, this,
											  boost::asio::placeholders::error,
											  boost::asio::placeholders::bytes_transferred));
}

void TcpHandler::ReadUntilHandler(const boost::system::error_code &/*ec*/, const std::size_t &/*read_bytes*/)
{
}

} // namespace ipc
} // namespace fastlink
