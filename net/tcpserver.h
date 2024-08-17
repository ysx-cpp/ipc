/*
 * @file tcpserver.h
 * @author Songxi Yang
 * @mail ysx-cpp@gmail.com
 * @github https://github.com/ysx-cpp
 * @date Oct 08 2019
 */
#ifndef NET_TCPSERVER_H
#define NET_TCPSERVER_H

#include "connectionpool.h"
#include "application.h"

namespace ipc {
namespace net {

class TcpServer : public ConnectionPool
{   
public:
    TcpServer(boost::asio::io_context &ioc, const std::string &host, unsigned short port);
    ~TcpServer() override;

    void Start();
    void Stop();
    
    boost::asio::io_context &io_context() { return io_context_; }

protected:
    void DoAccept();
    void OnAccept(ConnectionPtr connection,  const boost::system::error_code &ec);

private:
    boost::asio::io_context &io_context_;
    boost::asio::ip::tcp::acceptor acceptor_;
};

} // namespace net
} // namespace ipc
#endif // NET_TCPSERVER_H
