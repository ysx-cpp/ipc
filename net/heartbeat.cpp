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
#include "connection.h"


namespace fastlink {
namespace net {

using namespace boost::asio;
using namespace boost::posix_time;

Heartbeat::Heartbeat(boost::asio::io_context &ioc)
    : timer_(ioc)
{
}

void Heartbeat::Run()
{
	try
	{
		timer_.expires_from_now(boost::posix_time::seconds(this->expire()));
		timer_.async_wait(boost::bind(&Heartbeat::TimerHandle, this, placeholders::error));
	}
	catch (const boost::system::system_error &e)
	{
		std::cerr << e.what() << std::endl;
	}
}

void Heartbeat::CheckPing()
{
	if (!Stopped())
	{
		try
		{
			timer_.expires_at(timer_.expires_at() + boost::posix_time::seconds(this->expire()));
			timer_.async_wait(boost::bind(&Heartbeat::TimerHandle, this, placeholders::error));
		}
		catch (const boost::system::system_error &e)
		{
			std::cerr << e.what() << std::endl;
		}
	}
}

void Heartbeat::TimerHandle(const boost::system::error_code &ec)
{
	if (!ec) 
		this->Stop();
}

} // namespace net
} // namespace fastlink
