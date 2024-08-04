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
	using ReceiveHandle = std::function<void(const std::string &data)>;
public:
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
#endif // WIN32

} // namespace shm
} // namespace ipc

#endif // ICP_ASYNC_MESSAGE_QUEUE_H
