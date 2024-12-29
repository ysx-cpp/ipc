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
#include <functional>
#include <queue>
#include <unordered_map>
#include <boost/coroutine2/coroutine.hpp>
#include "tcpclient.h"

namespace ipc {
namespace net {

using RequestCallBack = std::function<int (const std::string&)>;

struct RpcContext
{
    uint16_t cmd;
    std::string req;
    std::string rsp;
    RequestCallBack call;
};

class RpcClient : public TcpClient
{
    using CoroutineType = boost::coroutines2::coroutine<void>;

public:
    RpcClient(boost::asio::io_context &ioc);
    ~RpcClient() = default;

    int Request(uint16_t cmd, const std::string &req, std::string &rsp);
    void AsyncRequest(const RpcContext &context);

private:
	int OnReceveData(const PackagePtr package) override final;
    void OnSendData(const std::size_t& write_bytes)  override;
    int OnConnect() override;
    int OnDisconnect() override;


private:
    std::unordered_map<uint16_t, std::queue<RpcContext>> cmd2context_;
    std::unique_ptr<CoroutineType::pull_type> source_;
};

} //namespace net
} //namespace ipc

#endif //NET_RPC_CLIENT_H
