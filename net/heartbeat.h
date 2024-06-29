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
class Heartbeat : public boost::enable_shared_from_this<Heartbeat>
{
public:
    explicit Heartbeat(boost::asio::io_context &ioc);

public:
	// void Ping(boost::function<void()> handler);
	void Ping(std::shared_ptr<Connection> connection);
	void StartTimer();
	void UpdateTimer();
	void TimerHandle(const boost::system::error_code &ec);
	void Stop() { stopped_ = true; }
	bool Stopped() const { return stopped_.load(); }

private:
	int expire() const { return 10; }

    boost::asio::deadline_timer server_timer_;
    boost::asio::deadline_timer clinet_timer_;
	std::atomic<bool> stopped_;
};

} // namespace net
} // namespace ipc

#endif // NET_HEARBEAT_H




