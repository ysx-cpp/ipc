#include "sendassist.h"
#include <memory>
#include "envelope.pb.h"
#include "streambuffer.hpp"
#include "utils.hpp"

namespace ipc {
namespace messages {

static const size_t HEAD_SIZE = sizeof(size_t);

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

        // LOG(INFO) << "init buuffer.size:" << buffer.size();

        auto mutable_buffer = buffer.prepare(message.size());
        std::memcpy(reinterpret_cast<char *>(mutable_buffer.data()),
                     reinterpret_cast<char *>(message.data()),
                     message.size());

        // LOG(INFO) << "prepare buuffer.size:" << buffer.size();
        buffer.commit(message.size());
        LOG(INFO) << "commit buuffer.size:" << buffer.size();
        return true;
    }
    catch (zmq::error_t &e)
    {
        LOG(ERROR) << (e.what());
    }
    return false;
}

void EncodeHead(size_t data_size, StreamBuffer& buffer)
{
    size_t packet_size = HEAD_SIZE + data_size;
    auto mutable_buffer = buffer.prepare(HEAD_SIZE);
    std::memcpy(reinterpret_cast<char *>(mutable_buffer.data()),
                 reinterpret_cast<char *>(&packet_size),
                 HEAD_SIZE);

    buffer.commit(HEAD_SIZE);
}

bool Encode(const ::google::protobuf::Message& message, StreamBuffer& buffer) 
{
    std::string data;
    if (!message.SerializeToString(&data))
        return false;

    EncodeHead(data.size(), buffer);

    // LOG(INFO) << "init buuffer.size:" << buffer.size();
    auto mutable_buffer = buffer.prepare(data.size());
    std::copy(data.begin(), data.end(), reinterpret_cast<char *>(mutable_buffer.data()));
    // LOG(INFO) << "prepare buuffer.size:" << buffer.size();
    buffer.commit(data.size());
    LOG(INFO) << "commit buuffer.size:" << buffer.size();

    return message.SerializeToArray(mutable_buffer.data(), mutable_buffer.size());
}

bool Decode(StreamBuffer& buffer, ::google::protobuf::Message &message)
{
    if (buffer.size() < sizeof(size_t))
    {
        LOGERROR << "buffer size:" << buffer.size();
        return false;
    }

    auto data = reinterpret_cast<const char*>(buffer.data().data());
    auto packet_size = reinterpret_cast<const size_t*>(data);

    if (buffer.size() < *packet_size)
    {
        LOGERROR << "buffer size:" << buffer.size() << " packet size:" << packet_size;
        return false;
    }

    if (!message.ParseFromArray(data + HEAD_SIZE, *packet_size - HEAD_SIZE))
    {
        LOGERROR << "buffer size:" << buffer.size() << " packet size:" << packet_size << " head_size:" << HEAD_SIZE;
        return false;
    }

    buffer.consume(*packet_size);

    return true;
}

std::string ParseHost(const std::string& protocol, const std::string& ip, int port)
{
    std::string addr;
    addr.append(protocol).append("://").append(ip).append(":").append(std::to_string(port));
    return addr;
}

} //namespace ipc
} //namespace messages