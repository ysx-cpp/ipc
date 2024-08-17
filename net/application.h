/*
 * @file reactor.h
 * @author Songxi Yang
 * @mail ysx-cpp@gmail.com
 * @github https://github.com/ysx-cpp
 * @date Oct 08 2019
 */
#ifndef NET_APPLICATION_H
#define NET_APPLICATION_H

#include <memory>
#include <boost/noncopyable.hpp>
#include <boost/thread.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/io_service_strand.hpp>
#include <boost/thread/detail/singleton.hpp>

namespace ipc {
namespace net {

class Application : private boost::noncopyable
{
public:
	Application() = default;
	~Application() = default;

    void Run();
    void RunThreadPool();
    void RunThreads(const int &thread_num);

	boost::asio::io_context &io_context() { return io_context_; }
    static void Signalhandle(const boost::system::error_code& err, int signal);

private:
	boost::asio::io_context io_context_;
    boost::thread_group thread_group_;
};

using ApplicationSingle = boost::detail::thread::singleton<Application>;

} // namespace net
} // namespace ipc

#endif //NET_APPLICATION_H
