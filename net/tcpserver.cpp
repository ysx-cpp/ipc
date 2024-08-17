/*
 * @file tcpserver.cpp
 * @author Songxi Yang
 * @mail ysx-cpp@gmail.com
 * @github https://github.com/ysx-cpp
 * @date Oct 08 2019
 */
#include "tcpserver.h"
#include <iostream>
#include <assert.h>
#include <boost/bind.hpp>
#include <boost/make_shared.hpp>
#include <boost/asio/ip/tcp.hpp>

namespace ipc {
namespace net {

using namespace std;
using namespace boost::asio;

TcpServer::TcpServer(boost::asio::io_context &ioc, const std::string & host, unsigned short port) : 
    io_context_(ioc),
    acceptor_(ioc, ip::tcp::endpoint(ip::address::from_string(host), port))
{
}

TcpServer::~TcpServer()
{
    Stop();
}

void TcpServer::Start()
{
    DoAccept();
}

void TcpServer::Stop()
{
    acceptor_.cancel();
    acceptor_.close();
    RemoveAllConnection();
}

void TcpServer::DoAccept()
{
    auto connection = CreateConnection(io_context_, this);
    acceptor_.async_accept(connection->socket_, boost::bind(&TcpServer::OnAccept, this, connection, boost::placeholders::_1));
}

void TcpServer::OnAccept(ConnectionPtr connection, const boost::system::error_code &ec)
{  
    int fd = connection->impl_.GetSocketFD();
    NET_LOGERR("ACCEPT SUCC error_code:" << ec << " fd:" << fd);
    if (ec)
    {
        std::cout << "delete session" << std::endl;    
        return connection->Stop();
    }
    DoAccept();

    //Start recive
    connection->Start();
}

} // namespace net
} // namespace ipc
