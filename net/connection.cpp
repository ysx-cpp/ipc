/*
 * @file connection.cpp
 * @author Songxi Yang
 * @mail ysx-cpp@gmail.com
 * @github https://github.com/ysx-cpp
 * @date Oct 08 2019
 */
#include "connection.h"
#include <iostream>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/enable_shared_from_this.hpp>
#include "connectionpool.h"

namespace ipc {
namespace net {

Connection::Connection(boost::asio::io_context &ioc) :
	TcpHandler(ioc),
	connction_pool_(nullptr),
	heartbeat_(new Heartbeat(ioc))
{
}

void Connection::Start()
{
    if (connction_pool_)
	{
        connction_pool_->AddConnection(ShaerdSelf());
	}

	set_connected(true);
    ReadSome();
}

void Connection::Stop()
{
	set_connected(false);

	if (connction_pool_)
		connction_pool_->RemoveConnection(ShaerdSelf());
}

void Connection::SetConnectionPool(ConnectionPool* conn_pool)
{
	connction_pool_ = conn_pool;
}

void Connection::EnableHeartbeat()
{
	heartbeat_->Run();
}

int Connection::OnHeartbeat()
{
	if (heartbeat_->Stopped())
	{
		Close();
		return boost::system::errc::timed_out;
	}
	else
		heartbeat_->CheckPing();

	return 0;
}

bool Connection::Connect(const std::string &host, unsigned short port)
{
	boost::system::error_code ec;
	boost::asio::ip::tcp::endpoint ep(boost::asio::ip::address::from_string(host), port);
	socket_.connect(ep, ec);
	return !ec;
}

void Connection::Send(Package& pkg)
{
	pkg.set_seq(send_seq_);
	pkg.Encode();
    WriteSome(pkg.data());
}

std::shared_ptr<Connection> Connection::ShaerdSelf()
{
	return std::dynamic_pointer_cast<Connection>(shared_from_this());
}

void Connection::Complete(const ByteArrayPtr data)
{
	auto package = std::make_shared<Package>();
	package->Decode(*data);

	if (package->seq() != recv_seq_)
		return;

	++recv_seq_;
    if (connction_pool_)
        connction_pool_->OnReceveData(package, std::dynamic_pointer_cast<Connection>(shared_from_this()));
}

void Connection::Successfully(const std::size_t& write_bytes)
{
	++send_seq_;
	if (connction_pool_)
		connction_pool_->OnSendData(write_bytes);
}

void Connection::Disconnect()
{
	heartbeat_->Stop();
	this->Stop();
}

} // namespace net
} // namespace ipc
