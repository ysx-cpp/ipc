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

namespace fastlink {
namespace ipc {

using namespace std;
using namespace boost::asio;

TcpServer::TcpServer(const std::string & host, unsigned short port) : 
    acceptor_(app_.io_context(), 
              ip::tcp::endpoint(ip::address::from_string(host), port))
{
}

TcpServer::~TcpServer()
{
    Stop();
}

void TcpServer::Start()
{
    DoAccept();
    app_.RunThreadPool();
}

void TcpServer::StartThreadPool()
{
    DoAccept();
    app_.Run();
}

void TcpServer::Stop()
{
    acceptor_.cancel();
    acceptor_.close();
    RemoveAllConnection();
}

void TcpServer::DoAccept()
{
    auto connection = CreateConnection(app_.io_context());
    //connection->SetSendBuffSize(8192);
    //connection->SetReciveBuffSize(8192);
    
    connection->SetManager(this);
    acceptor_.async_accept(connection->socket_, boost::bind(&TcpServer::OnAccept, this, connection, _1));
}

void TcpServer::OnAccept(ConnectionPtr connection, const boost::system::error_code &ec)
{  
    int fd = connection->socketfd();
    std::cout << "OnAccept error_code:" << ec << " fd:" << fd << std::endl;
    if (ec)
    {
        std::cout << "delete session" << std::endl;    
        return connection->Stop();
    }
    DoAccept();

    //Start recive
    connection->Start();
}

} // namespace ipc
} // namespace fastlink
