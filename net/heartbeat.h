/*
 * @file heartbeat.h
 * @author Songxi Yang
 * @mail ysx-cpp@gmail.com
 * @github https://github.com/ysx-cpp
 * @date Oct 08 2019
 */
#ifndef NET_HEARBEAT_H
#define NET_HEARBEAT_H

#include <boost/asio/io_service.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

namespace fastlink {
namespace ipc {

class Heartbeat : public boost::enable_shared_from_this<Heartbeat>
{
public:
    explicit Heartbeat(boost::asio::io_context &ioc);

public:
	void Run();
	void CheckPing();
	void TimerHandle(const boost::system::error_code &ec);
	void Stop() { stopped_ = true; }
	bool Stopped() const { return stopped_; }

private:
	int expire() const { return 10; }

    boost::asio::deadline_timer timer_;
	bool stopped_ = false;
	boost::posix_time::ptime last_ping_;
};

} // namespace ipc
} // namespace fastlink

#endif // NET_HEARBEAT_H




