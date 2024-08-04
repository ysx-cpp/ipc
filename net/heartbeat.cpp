/*
 * @file heartbeat.h
 * @author Songxi Yang
 * @mail ysx-cpp@gmail.com
 * @github https://github.com/ysx-cpp
 * @date Oct 08 2019
 */
#include "heartbeat.h"
#include <iostream>
#include <fstream>
#include <boost/bind.hpp>
#include <boost/asio/placeholders.hpp>
#include "connection.h"
#include "logdefine.h"

namespace ipc {
namespace net {

using namespace boost::asio;
using namespace boost::posix_time;

Heartbeat::Heartbeat(boost::asio::io_context &ioc) :
request_timer_(ioc),
reply_timer_(ioc),
stopped_(false)
{
	last_ping_ = boost::posix_time::second_clock::local_time();
}

void Heartbeat::Ping(std::shared_ptr<Connection> connection, const boost::system::error_code &ec)
{
	if (!connection)
	{
		NET_LOGERR("connection is nullptr");
		return;
	}

	if (Stopped()) 
	{
		NET_LOGERR("Heartbeat Stopped");
		return;
	}

	if (!connection->Connected())
	{
		NET_LOGERR("Disconnected!");
		return;
	}

    request_timer_.expires_from_now(boost::posix_time::seconds(5));
    request_timer_.async_wait(boost::bind(&Heartbeat::Ping, shared_from_this(), connection, placeholders::error));
}

void Heartbeat::Ping(std::shared_ptr<Connection> connection)
{
	if (!connection)
	{
		NET_LOGERR("connection is nullptr");
		return;
	}

	if (Stopped())
	{
		NET_LOGERR("Heartbeat Stopped");
		return;
	}

	if (!connection->Connected())
	{
		NET_LOGERR("Disconnected!");
		return;
	}

	auto pself = shared_from_this();
	auto lamb = [pself, connection](const boost::system::error_code &ec)
	{
		if (!connection)
		{
			NET_LOGERR("connection is nullptr");
			return;
		}
		if (!connection->Connected())
		{
			NET_LOGERR("Disconnected!");
			return;
		}

		Package pkg;
		pkg.set_cmd(0);
		connection->SendData(pkg, "ping");

		if (pself)
			pself->Ping(connection);
	};

	request_timer_.expires_from_now(boost::posix_time::seconds(5));
	request_timer_.async_wait(lamb);
}

void Heartbeat::StartTimer()
{
	reply_timer_.expires_from_now(boost::posix_time::seconds(this->expire()));
	reply_timer_.async_wait(boost::bind(&Heartbeat::TimerHandle, shared_from_this(), placeholders::error));
}

void Heartbeat::UpdateTimer()
{
	if (Stopped()) 
	{
		NET_LOGERR("Heartbeat Stopped");
		return;
	}
	last_ping_ = boost::posix_time::second_clock::local_time();

	// reply_timer_.expires_at(reply_timer_.expires_at() + boost::posix_time::seconds(this->expire()));
	// reply_timer_.async_wait(boost::bind(&Heartbeat::TimerHandle, shared_from_this(), placeholders::error));
}

void Heartbeat::TimerHandle(const boost::system::error_code &ec)
{
	if (Stopped()) 
	{
		NET_LOGERR("Heartbeat Stopped ec error message:" << ec.message());
		return;
	}
	
	auto now = boost::posix_time::second_clock::local_time();
	if ((now - last_ping_) > boost::posix_time::seconds(this->expire()))
	{
		NET_LOGERR("Heartbeat time out ec error message:" << ec.message());
		return Stop();
	}
	reply_timer_.expires_from_now(boost::posix_time::seconds(this->expire()));
	reply_timer_.async_wait(boost::bind(&Heartbeat::TimerHandle, shared_from_this(), placeholders::error));
}

void Heartbeat::Stop()
{
	boost::system::error_code ec;
	stopped_ = true;
	reply_timer_.cancel(ec);
	request_timer_.cancel(ec);
}

} // namespace net
} // namespace ipc
