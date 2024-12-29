#include "rpcclient.h"
#include <functional>
#include <string>
#include <boost/asio.hpp>

namespace ipc {
namespace net {

RpcClient::RpcClient(boost::asio::io_context &ioc) :
    TcpClient(ioc)
{
    source_.reset(new CoroutineType::pull_type([this](CoroutineType::push_type &yield) {
        try
        {
            yield();
            for (auto &it : cmd2context_)
            {
                if (it.second.empty())
                    continue;

                auto &context = it.second.front();
                PackagePtr pkg = std::make_shared<Package>();
                pkg->set_cmd(context.cmd);
                SendPackage(pkg, context.req);
                NET_LOGINFO("Step1 SendPackage:" << context.req);
                yield();

                NET_LOGINFO("Step3 rsp:" << context.rsp);
                context.call(context.rsp);
                it.second.pop();
            }
        }
        catch(const std::exception& e)
        {
            NET_LOGERR(e.what());
        }
    }));
}

int RpcClient::Request(uint16_t cmd, const std::string &req, std::string &rsp)
{
    CoroutineType::pull_type source([&](CoroutineType::push_type& sink) {
        RpcContext context;
        context.cmd = cmd;
        context.req = req;
        context.rsp = rsp;
        // contex.call = ;

        auto &queue_ctx = cmd2context_[context.cmd];
        queue_ctx.push(context);
        (*source_)();
    });

    return 0;
}

void RpcClient::AsyncRequest(const RpcContext &context)
{
    auto &queue_ctx = cmd2context_[context.cmd];
    queue_ctx.push(context);
    (*source_)();
}

int RpcClient::OnReceveData(const PackagePtr package)
{
    NET_LOGINFO("Step2 OnReceveData:" << package->pdata());
    if (package && source_)
    {
        auto it = cmd2context_.find(package->cmd());
        if (it != cmd2context_.end())
        {
            auto &context = it->second.front();
            context.rsp.assign(package->data().begin(), package->data().end());
            (*source_)();
        }
    }
    return 0;
}

void RpcClient::OnSendData(const std::size_t& write_bytes)
{
}

int RpcClient::OnConnect()
{
    return 0;
}

int RpcClient::OnDisconnect()
{
    Stop();
    return 0;
}

} //namespace net
} //namespace ipc
