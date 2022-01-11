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

namespace fastlink {
namespace net {

TcpClient::TcpClient(boost::asio::io_context &ioc)
    : Connection(ioc),
    ioc_(ioc),
    timer_(ioc),
    timer_conn_(ioc)
{
}

void TcpClient::CreateConnect(const std::string &host, uint16_t port, int timeout /*=200*/)
{
    option_.host = host;
    option_.port = port;
    option_.timeout_ms = timeout;
}

void TcpClient::Ping()
{
    timer_.expires_from_now(boost::posix_time::seconds(this->expire()));
    timer_.async_wait(boost::bind(&TcpClient::Ping, this));

    // this->ioc_.post(boost::bind(&TcpClient::Ping, this));

    if (connected())
        SendData("1");
}

void TcpClient::SendData(const std::string &msg)
{
    if (!connected())
    {
        if (!Connect(option_.host, option_.port))
        {
            timer_conn_.expires_from_now(boost::posix_time::milliseconds(option_.timeout_ms));
            timer_conn_.async_wait(boost::bind(&TcpClient::SendData, this, msg));

            std::cerr << "connect " << option_.host << ":" << option_.port << " timeout" << std::endl;
            return;
        }
        else 
        {
            Start();
        }
    }

    ByteArray data;
    data.assign(msg.begin(), msg.end());
    Send(data);
}

void TcpClient::Disconnect()
{
    Close();
    Connection::Disconnect();
}

} // namespace net
} // namespace fastlink
