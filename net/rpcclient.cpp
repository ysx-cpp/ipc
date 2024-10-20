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
}

int RpcClient::Request(const std::string &req, std::string &rsp)
{
     source_.reset(new CoroutineType::pull_type([&req, &rsp, this](CoroutineType::push_type &yield) {
        auto pos = req.find_first_of(":");
        if (pos == std::string::npos)
        {
            return;
        }

        auto frist = req.substr(0, pos);
        auto cmd = std::strtol(frist.c_str(), nullptr, 10);
        auto msg = req.substr(pos + 1);

        PackagePtr pkg = std::make_shared<Package>();
        pkg->set_cmd(cmd);
        SendPackage(pkg, msg);
        NET_LOGINFO("Step1 SendPackage:" << msg);

        yield(); 

        NET_LOGINFO("Step3 rsp:" << rsp_);
        rsp.assign(rsp_.begin(), rsp_.end());
    }));

    return 0;
}

int RpcClient::OnReceveData(const PackagePtr package)
{
    NET_LOGINFO("Step2 OnReceveData:" << package->pdata());
    if (package && source_)
    {
        rsp_.assign(package->data().begin(), package->data().end());
        (*source_)();
    }
    return 0;
}

} //namespace net
} //namespace ipc
