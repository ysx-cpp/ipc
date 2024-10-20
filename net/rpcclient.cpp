#include "rpcclient.h"
#include <functional>
#include <string>
#include <boost/asio.hpp>
#include <iostream>
#include <thread>

namespace ipc {
namespace net {

RpcClient::RpcClient(boost::asio::io_context &ioc) :
    TcpClient(ioc)
{
    source_.reset(new CoroutineType::pull_type([this](CoroutineType::push_type &yield) {
        
        yield(); 
        for (auto &it : cmd2context_)
        {
            if (it.second.empty()) continue;

            auto &ctx = it.second.front();
            PackagePtr pkg = std::make_shared<Package>();
            pkg->set_cmd(ctx.cmd);
            SendPackage(pkg, ctx.req);
            NET_LOGINFO("Step1 SendPackage:" << ctx.req);

            yield(); 
            
            NET_LOGINFO("Step3 rsp:" << ctx.rsp);
            ctx.call(ctx.rsp);
            it.second.pop();
        }
    }));
}

void RpcClient::AsyncRequest(const RpcContext &ctx)
{
    auto &queue_ctx = cmd2context_[ctx.cmd];
    queue_ctx.push(ctx);
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
            auto &ctx = it->second.front();
            ctx.rsp.assign(package->data().begin(), package->data().end());
            (*source_)();
        }
    }
    return 0;
}

} //namespace net
} //namespace ipc
