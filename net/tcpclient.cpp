/*
 * @file tcpclient.cpp
 * @author Songxi Yang
 * @mail ysx-cpp@gmail.com
 * @github https://github.com/ysx-cpp
 * @date Oct 08 2019
 */
#include "tcpclient.h"
#include <iostream>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include "package.h"

namespace ipc {
namespace net {

TcpClient::TcpClient(boost::asio::io_context &ioc)
    : Connection(ioc),
    ioc_(ioc),
    timer_(ioc)
{
}

void TcpClient::CreateConnect(const std::string &host, uint16_t port, int timeout /*=200*/)
{
    option_.host = host;
    option_.port = port;
    option_.timeout_ms = timeout;
}

void TcpClient::SendPackage(PackagePtr package, const std::string &msg)
{
    try
    {
        if (!Connected())
        {
            if (!Connect(option_.host, option_.port))
            {
                timer_.expires_from_now(boost::posix_time::milliseconds(option_.timeout_ms));
                timer_.async_wait(boost::bind(&TcpClient::SendPackage, this, package, msg));

                std::cerr << "connect " << option_.host << ":" << option_.port << " timeout" << std::endl;
                return;
            }
            else
            {
                Start();
            }
        }
        SendData(*package, msg);
    }
    catch (const std::exception &e)
    {
        NET_LOGERR(e.what());
    }
    catch (...)
    {
        NET_LOGERR("Unknown error");
    }
}

} // namespace net
} // namespace ipc
