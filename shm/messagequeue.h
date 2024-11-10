/*
 * @file messagequeue.h
 * @author Songxi Yang
 * @mail ysx-cpp@gmail.com
 * @github https://github.com/ysx-cpp
 * @date Oct 08 2021
 */
#ifndef ICP_MESSAGE_QUEUE_H
#define ICP_MESSAGE_QUEUE_H

#include <string>
#include <boost/interprocess/ipc/message_queue.hpp>
#include <boost/asio/streambuf.hpp>

namespace ipc {
namespace shm {

class MessageQueue
{
public:
	MessageQueue(const std::string & name, unsigned int max_num_msg);
	MessageQueue(const std::string & name);
	~MessageQueue();
	bool TrySend(const std::string &data);
	bool TryReceive(std::string &data);
	size_t Send(const std::string &data);
	size_t Receive(std::string &data);
	
private:
	enum { MAX_MSG_SIZE = 10 * 1024 * 1024}; //10M
	boost::interprocess::message_queue mq_;
	boost::asio::streambuf recv_buff_;
	boost::asio::streambuf send_buff_;
};

} // namespace shm
} // namespace ipc

#endif // ICP_MESSAGE_QUEUE_H
