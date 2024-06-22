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

namespace ipc {
namespace net {

using namespace boost::asio;
using namespace boost::posix_time;

Heartbeat::Heartbeat(boost::asio::io_context &ioc) :
server_timer_(ioc),
clinet_timer_(ioc)
{
}

void Heartbeat::Ping(boost::function<void()> handler)
{
	auto lamb = [=](const boost::system::error_code &ec) { 
		handler(); 
		Ping(handler);
	};

    clinet_timer_.expires_from_now(boost::posix_time::seconds(5));
    clinet_timer_.async_wait(lamb);
}

void Heartbeat::StartTimer()
{
	try
	{
		server_timer_.expires_from_now(boost::posix_time::seconds(this->expire()));
		server_timer_.async_wait(boost::bind(&Heartbeat::TimerHandle, this, placeholders::error));
	}
	catch (const boost::system::system_error &e)
	{
		std::cerr << e.what() << std::endl;
	}
	catch (...)
	{
		std::cerr << "Unknown error" << std::endl;
	}
}

void Heartbeat::UpdateTimer()
{
	if (!Stopped())
	{
		try
		{
			server_timer_.expires_at(server_timer_.expires_at() + boost::posix_time::seconds(this->expire()));
			server_timer_.async_wait(boost::bind(&Heartbeat::TimerHandle, this, placeholders::error));
		}
		catch (const boost::system::system_error &e)
		{
			std::cerr << e.what() << std::endl;
		}
		catch (...)
		{
			std::cerr << "Unknown error" << std::endl;
		}
	}
}

void Heartbeat::TimerHandle(const boost::system::error_code &ec)
{
	if (!ec) this->Stop();
}

} // namespace net
} // namespace ipc
