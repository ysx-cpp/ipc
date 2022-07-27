#ifndef SEND_ASSIST_HPP
#define SEND_ASSIST_HPP
#include <memory>
#include <glog/logging.h>
#include "envelope.pb.h"
#include "streambuffer.hpp"

namespace ipc {
namespace messages {

bool SendMessage(std::unique_ptr<zmq::socket_t>& socket, const StreamBuffer& buffer);
bool RecvMessage(std::unique_ptr<zmq::socket_t> &socket, StreamBuffer& buffer);
void EncodeHead(size_t data_size, StreamBuffer& buffer);
bool Encode(const ::google::protobuf::Message& message, StreamBuffer& buffer);
bool Decode(StreamBuffer& buffer, ::google::protobuf::Message &message);
std::string ParseHost(const std::string& protocol, const std::string& ip, int port);

} //namespace ipc
} //namespace messages

#endif //SEND_ASSIST_HPP