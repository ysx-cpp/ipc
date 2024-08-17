/*
 * @file reactor.cpp
 * @author Songxi Yang
 * @mail ysx-cpp@gmail.com
 * @github https://github.com/ysx-cpp
 * @date Oct 08 2019
 */
#include "application.h"
#include <iostream>
#include <exception>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/signals2.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/make_shared.hpp>
#include <boost/make_unique.hpp>

namespace ipc {
namespace net {

using namespace boost::asio;

void Application::Run()
{
    try
	{
		boost::asio::signal_set sigset(this->io_context(), SIGINT, SIGTERM);
		sigset.async_wait(Application::Signalhandle);
		io_context_.run();
        std::cout << "thread id: " << boost::this_thread::get_id() << std::endl;
	}
	catch (boost::system::system_error &e)
	{
		std::cerr << e.what() << std::endl;
        abort();
	}
}

void Application::RunThreadPool()
{
    try
    {
        std::size_t thread_num = detail::thread::hardware_concurrency() * 2;
        thread_num = thread_num ? thread_num : 2;
        for (std::size_t i = 0; i < thread_num; ++i)
        {
            thread_group_.create_thread(boost::bind(&Application::Run, this));
        }
        thread_group_.join_all();
    }
    catch(const std::exception &e)
    {
        std::cout << "runThreadPool thow " << e.what() << std::endl;
    }
}

void Application::RunThreads(const int &thread_num)
{
    std::vector<boost::shared_ptr<boost::thread>> thread_group;
    for (int i = 0; i < thread_num; ++i)
    {
        boost::function<void (void)> f = boost::bind(&Application::Run, this);
        auto thread = boost::make_shared<boost::thread>(f);
        thread_group.push_back(thread);
        std::cout << "thread id: " << thread->get_id() << std::endl;
    }
    for (auto &iter : thread_group)
    {
        if (iter->joinable())
            iter->join();
    }
}

void Application::Signalhandle(const boost::system::error_code& /*err*/, int signal)
{
	switch (signal) {
	case SIGINT:
		std::cout << "SIGNINT" << std::endl;
		break;
	case SIGTERM:
		std::cout << "SIGNTERM" << std::endl;
		break;
	default:
		break;
	}
}

} // namespace net
} // namespace ipc
