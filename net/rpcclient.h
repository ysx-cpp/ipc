/*
 * @file connection.h
 * @author Songxi Yang
 * @mail ysx-cpp@gmail.com
 * @github https://github.com/ysx-cpp
 * @date 20 Oct 2024
 */
#ifndef NET_RPC_CLIENT_H
#define NET_RPC_CLIENT_H
#include <memory>
#include <boost/coroutine2/coroutine.hpp>
#include "tcpclient.h"

namespace ipc {
namespace net {

class RpcClient : public TcpClient
{
    using CoroutineType = boost::coroutines2::coroutine<void>;
public:
    RpcClient(boost::asio::io_context &ioc);
    ~RpcClient() = default;

    int Request(const std::string &req, std::string &rsp);

private:
	int OnReceveData(const PackagePtr package) override final;

private:
    std::unique_ptr<CoroutineType::pull_type> source_;
    std::string rsp_;
};

} //namespace net
} //namespace ipc

#endif //NET_RPC_CLIENT_H