/*
 * @file messagequeue.h
 * @author Songxi Yang
 * @mail ysx-cpp@gmail.com
 * @github https://github.com/ysx-cpp
 * @date Oct 08 2024
 */
#include "asyncmessagequeue.h"
#ifndef WIN32
#include <sys/eventfd.h>
#include <string>
#include <iostream>
#include <thread>
#include <boost/bind.hpp>

namespace ipc {
namespace shm {

AsyncMessageQueue::AsyncMessageQueue(boost::asio::io_context &io_context, const std::string &queue_name) : 
io_context_(io_context),
event_fd_(eventfd(0, 0)),
stream_descriptor_(io_context, event_fd_),
queue_(queue_name)
{
}

AsyncMessageQueue::~AsyncMessageQueue()
{
	::close(event_fd_);
}

void AsyncMessageQueue::AsyncReceive(ReceiveHandle handle)
{
	stream_descriptor_.async_read_some(boost::asio::null_buffers(), boost::bind(&AsyncMessageQueue::OnAsyncReceive, this, std::move(handle)));
}

void AsyncMessageQueue::OnAsyncReceive(ReceiveHandle handle)
{
	eventfd_t value;
	eventfd_read(event_fd_, &value);

	// unsigned int priority;
	// boost::interprocess::message_queue::size_type received_size;
	// int number;

	// if (message_queue_.try_receive(&number, sizeof(number), received_size, priority))
	// {
	// 	std::cout << "Received: " << number << std::endl;
	// }
	std::string data;
	queue_.Receive(data);

	handle(data);

	AsyncReceive(std::move(handle));
}

void AsyncMessageQueue::AsyncSend(const std::string &data)
{
	queue_.Send(data);
	eventfd_write(event_fd_, 1);
}
#endif // WIN32

} // namespace shm
} // namespace ipc

