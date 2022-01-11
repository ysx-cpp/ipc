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
#include "connectionmgr.h"

namespace fastlink {
namespace ipc {

Connection::Connection(boost::asio::io_context &ioc) :
	TcpHandler(ioc),
	manager_(nullptr),
	heartbeat_(new Heartbeat(ioc))
{
}

void Connection::Start()
{
    if (manager_)
	{
        manager_->AddConnection(ShaerdSelf());
	}

	set_connected(true);
    ReadSome();
}

void Connection::Stop()
{
	set_connected(false);

	if (manager_)
		manager_->RemoveConnection(ShaerdSelf());
}

void Connection::SetManager(ConnectionMgr *manager)
{
	manager_ = manager;
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

void Connection::Send(ByteArray &data)
{
    Head head;
    head.Encode(data);
    Write(data);
}

std::shared_ptr<Connection> Connection::ShaerdSelf()
{
	return std::dynamic_pointer_cast<Connection>(shared_from_this());
}

void Connection::Complete(const ByteArrayPtr data)
{
    if (manager_)
        manager_->OnReceveData(data, std::dynamic_pointer_cast<Connection>(shared_from_this()));
}

void Connection::Disconnect()
{
	heartbeat_->Stop();
	this->Stop();
}

} // namespace ipc
} // namespace fastlink
