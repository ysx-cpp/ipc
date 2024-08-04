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
#include <boost/function.hpp>

namespace ipc {
namespace net {

class Connection;
class Heartbeat : public std::enable_shared_from_this<Heartbeat>
{
public:
    explicit Heartbeat(boost::asio::io_context &ioc);

public:
	void Ping(std::shared_ptr<Connection> connection, const boost::system::error_code &ec);
	void Ping(std::shared_ptr<Connection> connection);
	void StartTimer();
	void UpdateTimer();
	void TimerHandle(const boost::system::error_code &ec);
	bool Stopped() const { return stopped_; }
	void Stop();

private:
	int expire() const { return 10; }

    boost::asio::deadline_timer request_timer_;
    boost::asio::deadline_timer reply_timer_;
	std::atomic<bool> stopped_;
	boost::posix_time::ptime last_ping_;
};

} // namespace net
} // namespace ipc

#endif // NET_HEARBEAT_H




