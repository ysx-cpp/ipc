/*
 * @file messagequeue.cpp
 * @author Songxi Yang
 * @mail ysx-cpp@gmail.com
 * @github https://github.com/ysx-cpp
 * @date Oct 08 2021
 */
#include "messagequeue.h"
#include <iostream>
#include <boost/interprocess/mapped_region.hpp>

namespace ipc {
namespace shm {

using namespace boost::interprocess;

MessageQueue::MessageQueue(const std::string & name, unsigned int max_num_msg)
	: mq_(open_or_create, name.c_str(), max_num_msg, MAX_MSG_SIZE)
{
}

MessageQueue::MessageQueue(const std::string & name)
    : mq_(open_or_create, name.c_str(), 1, MAX_MSG_SIZE)
{
}

MessageQueue::~MessageQueue()
{
}

bool MessageQueue::TrySend(const std::string & data)
{
	try
	{
		if (data.size() > send_buff_.max_size())
			return false;

		if (data.size() > mq_.get_max_msg_size())
			return false;

		unsigned int priority = 0;
		std::ostream os(&send_buff_);
    	os << data;
		
		if (!mq_.try_send(send_buff_.data().data(), send_buff_.size(), priority))
			return false;

		send_buff_.consume(send_buff_.size());

		return true;
	}
	catch (const interprocess_exception &e)
	{
		std::cout << e.what() << std::endl;
	}
	return false;
}

bool MessageQueue::TryReceive(std::string &data)
{
	unsigned int priority;
	boost::interprocess::message_queue::size_type received_size;
	boost::asio::mutable_buffer buffer = recv_buff_.prepare(mq_.get_max_msg_size());

	if (!mq_.try_receive(buffer.data(), mq_.get_max_msg_size(), received_size, priority))
	{
		std::cerr << "TryReceive error!" << std::endl;
		return false;
	}

	recv_buff_.commit(received_size);
	std::istream is(&recv_buff_);
	is >> data;
	
	return true;
}

size_t MessageQueue::Send(const std::string & data)
{
	try
	{
		if (data.size() > send_buff_.max_size())
			return 0;

		if (data.size() > mq_.get_max_msg_size())
			return 0;

		unsigned int priority = 0;
		boost::asio::mutable_buffer buffer = send_buff_.prepare(data.size());
		boost::asio::buffer_copy(buffer, boost::asio::buffer(data));
		
		mq_.send(buffer.data(), buffer.size(), priority);
		send_buff_.consume(send_buff_.size());

		return data.size();
	}
	catch (const interprocess_exception &e)
	{
		std::cout << e.what() << std::endl;
	}
	return 0;
}

size_t MessageQueue::Receive(std::string & data)
{
	try
	{
		unsigned int priority;
		boost::interprocess::message_queue::size_type recvd_size;
        boost::asio::mutable_buffer buffer = recv_buff_.prepare(mq_.get_max_msg_size());

		mq_.receive(buffer.data(), mq_.get_max_msg_size(), recvd_size, priority);

		recv_buff_.commit(recvd_size);
		auto p = reinterpret_cast<const char*>(buffer.data());
		data.assign(p, p + recvd_size);
		recv_buff_.consume(recvd_size);

		return recvd_size;
	}
	catch (const interprocess_exception &e)
	{
		std::cout << e.what() << std::endl;
	}
    return 0;
}

} // namespace shm
} // namespace ipc 
