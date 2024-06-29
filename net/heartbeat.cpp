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

namespace ipc {
namespace net {

using namespace boost::asio;
using namespace boost::posix_time;

Heartbeat::Heartbeat(boost::asio::io_context &ioc) :
server_timer_(ioc),
clinet_timer_(ioc),
stopped_(false)
{
}

void Heartbeat::Ping(std::shared_ptr<Connection> connection, const boost::system::error_code &ec)
{
	if (!connection)
	{
		std::cerr << __FILE__ << ":" << __LINE__ << ":" << __FUNCTION__ << "|" << "connection is nullptr" << std::endl;
		return;
	}

	if (Stopped()) 
	{
		std::cerr << __FILE__ << ":" << __LINE__ << ":" << __FUNCTION__ << "|" << "stoped" << std::endl;
		return;
	}

    clinet_timer_.expires_from_now(boost::posix_time::seconds(5));
    clinet_timer_.async_wait(boost::bind(&Heartbeat::Ping, shared_from_this(), connection, placeholders::error));
}

void Heartbeat::Ping(std::shared_ptr<Connection> connection)
{
	try
	{
		if (!connection)
		{
			std::cerr << __FILE__ << ":" << __LINE__ << ":" << __FUNCTION__ << "|" << "connection is nullptr" << std::endl;
			return;
		}

		if (Stopped())
		{
			std::cerr << __FILE__ << ":" << __LINE__ << ":" << __FUNCTION__ << "|" << "stoped" << std::endl;
			return;
		}

		auto pself = shared_from_this();
		auto lamb = [pself, connection](const boost::system::error_code &ec)
		{
			if (connection)
			{
				Package pkg;
				pkg.set_cmd(0);
				connection->SendData(pkg, "ping");
			}

			if (pself)
				pself->Ping(connection);
		};

		clinet_timer_.expires_from_now(boost::posix_time::seconds(5));
		clinet_timer_.async_wait(lamb);
	}
	catch (const boost::system::system_error &e)
	{
		std::cerr << e.what() << std::endl;
	}
	catch (...)
	{
		std::cerr << __FILE__ << ":" << __LINE__ << ":" << __FUNCTION__ << "|" << "Unknown error" << std::endl;
	}
}

void Heartbeat::StartTimer()
{
	try
	{
		server_timer_.expires_from_now(boost::posix_time::seconds(this->expire()));
		// server_timer_.async_wait(boost::bind(&Heartbeat::TimerHandle, this, placeholders::error));

		auto pself = shared_from_this();
		auto lamb = [pself](const boost::system::error_code &ec)
		{
			if (pself)
				pself->TimerHandle(ec);
		};
		server_timer_.async_wait(lamb);
	}
	catch (const boost::system::system_error &e)
	{
		std::cerr << e.what() << std::endl;
	}
	catch (...)
	{
		std::cerr << __FILE__ << ":" << __LINE__ << ":" << __FUNCTION__ << "|" << "Unknown error" << std::endl;
	}
}

void Heartbeat::UpdateTimer()
{
	if (Stopped()) return;

	try
	{
		server_timer_.expires_at(server_timer_.expires_at() + boost::posix_time::seconds(this->expire()));
		server_timer_.async_wait(boost::bind(&Heartbeat::TimerHandle, this, placeholders::error));

		auto pself = shared_from_this();
		auto lamb = [pself](const boost::system::error_code &ec)
		{
			if (pself)
				pself->TimerHandle(ec);
		};
		server_timer_.async_wait(lamb);
	}
	catch (const boost::system::system_error &e)
	{
		std::cerr << e.what() << std::endl;
	}
	catch (...)
	{
		std::cerr << __FILE__ << ":" << __LINE__ << ":" << __FUNCTION__ << "|" << "Unknown error" << std::endl;
	}
}

void Heartbeat::TimerHandle(const boost::system::error_code &ec)
{
	if (!ec) this->Stop();
}

} // namespace net
} // namespace ipc
