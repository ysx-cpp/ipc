/*
 * @file messagequeue.h
 * @author Songxi Yang
 * @mail ysx-cpp@gmail.com
 * @github https://github.com/ysx-cpp
 * @date Oct 08 2024
 */
#ifndef ICP_ASYNC_MESSAGE_QUEUE_H
#define ICP_ASYNC_MESSAGE_QUEUE_H

#ifndef WIN32
#include <string>
#include <functional>
#include <boost/asio.hpp>
#include "messagequeue.h"

namespace ipc {
namespace shm {

class AsyncMessageQueue
{
public:
	using ReceiveHandle = std::function<int(const std::string &data)>;

	AsyncMessageQueue(boost::asio::io_context &io_context, const std::string &queue_name);
	~AsyncMessageQueue();

	void AsyncReceive(ReceiveHandle handle);
	void AsyncSend(const std::string &data);

private:
	void OnAsyncReceive(ReceiveHandle handle);

private:
		boost::asio::io_context &io_context_;
		int event_fd_;
		boost::asio::posix::stream_descriptor stream_descriptor_;
		MessageQueue queue_;
};


} // namespace shm
} // namespace ipc
#endif // WIN32
#endif // ICP_ASYNC_MESSAGE_QUEUE_H
