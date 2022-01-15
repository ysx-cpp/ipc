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
	: mq_(create_only, name.c_str(), max_num_msg, MAX_MSG_SIZE)
{
}

MessageQueue::MessageQueue(const std::string & name)
    : mq_(create_only, name.c_str(), 1, MAX_MSG_SIZE)
{
}

MessageQueue::~MessageQueue()
{
}

size_t MessageQueue::Send(const std::string & data)
{
    size_t send_leng = 0;
    auto max_msg_size = static_cast<int>(mq_.get_max_msg_size());
	auto first = data.begin();
	for (unsigned int i = 0; i < mq_.get_num_msg(); ++i)
	{
        auto leng = std::min<int>(max_msg_size, static_cast<int>(data.end() - first));
		if (leng == 0)
			break;

        std::string tmp(first, first + leng);
        first += leng;
		send_leng += DoSend(tmp);
	}
	return send_leng;
}

size_t MessageQueue::DoSend(const std::string & data)
{
	auto leng = (data.size());
	try
	{
		if (data.size() > send_buff_.max_size())
			return leng;

		boost::asio::mutable_buffer buffer = send_buff_.prepare(data.size());
		std::copy(data.cbegin(), data.cend(), static_cast<unsigned char*>(buffer.data()));
		mq_.send(buffer.data(), buffer.size(), 0);
		send_buff_.consume(send_buff_.size());

		return send_buff_.size();
	}
	catch (const interprocess_exception &e)
	{
		std::cout << e.what() << std::endl;
	}
	return leng;
}

size_t MessageQueue::Receive(std::string & data)
{
    auto recvd_leng = 0u;
	for (unsigned int i = 0; i < mq_.get_num_msg(); ++i)
	{
		std::string tmp;
        recvd_leng += DoReceive(tmp);
        if (recvd_leng > 0)
            data.append(tmp);
	}
	assert(recvd_leng == data.size());
	return recvd_leng;
}

size_t MessageQueue::DoReceive(std::string & data)
{
	try
	{
        boost::asio::mutable_buffer buffer = recv_buff_.prepare(mq_.get_max_msg_size());
		size_t recvd_size;
		unsigned int priority;
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
