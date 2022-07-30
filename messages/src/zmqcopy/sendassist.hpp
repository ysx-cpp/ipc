#pragma once
#include <memory>
#include <glog/logging.h>
#include "zmq_config.h"
#include "envelope.pb.h"
#include "streambuffer.hpp"

namespace ipc {
namespace messages {

bool SendMessage(std::unique_ptr<zmq::socket_t>& socket, const StreamBuffer& buffer) 
{
    try 
    {
        socket->send(buffer.data());
        return true;
    } 
    catch (zmq::error_t& e) 
    {
        LOG(ERROR) << (e.what());
    }
    return false;
}

bool RecvMessage(std::unique_ptr<zmq::socket_t> &socket, StreamBuffer& buffer)
{
    try
    {
        zmq::message_t message;
        if (!socket->recv(message))
            return false;

        LOG(INFO) << "init buuffer.size:" << buffer.size();

        auto mutable_buffer = buffer.prepare(message.size());
        std::strncpy(reinterpret_cast<char *>(mutable_buffer.data()),
                     reinterpret_cast<char *>(message.data()),
                     message.size());

        LOG(INFO) << "prepare buuffer.size:" << buffer.size();
        buffer.commit(message.size());
        LOG(INFO) << "commit buuffer.size:" << buffer.size();
    }
    catch (zmq::error_t &e)
    {
        LOG(ERROR) << (e.what());
    }
    return false;
}

bool Encode(const ::google::protobuf::Message& message, StreamBuffer& buffer) 
{
    std::string data;
    if (!message.SerializeToString(&data))
        return false;

    LOG(INFO) << "init buuffer.size:" << buffer.size();
    auto mutable_buffer = buffer.prepare(data.size());
    std::copy(data.begin(), data.end(), reinterpret_cast<char *>(mutable_buffer.data()));

    LOG(INFO) << "prepare buuffer.size:" << buffer.size();
    buffer.commit(data.size());
    LOG(INFO) << "commit buuffer.size:" << buffer.size();

    return message.SerializeToArray(mutable_buffer.data(), mutable_buffer.size());
}

bool Decode(StreamBuffer& buffer, ::google::protobuf::Message &message)
{
    if (buffer.size() < sizeof(size_t))
        return false;

    auto data = reinterpret_cast<const char*>(buffer.data().data());
    auto data_size = reinterpret_cast<const size_t*>(data);

    if (buffer.size() < *data_size)
        return false;

    if (!message.ParseFromArray(data, *data_size))
        return false;

    buffer.consume(*data_size);

    return true;
}

std::string ParseHost(const std::string& protocol, const std::string& ip, int port)
{
    std::string addr;
    addr.append(protocol).append(ip).append(":").append(std::to_string(port));
    return addr;
}

} //namespace ipc
} //namespace messages