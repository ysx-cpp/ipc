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
    TcpServer(const std::string &host, unsigned short port);

    ~TcpServer() override;

    void Start();
    void StartThreadPool();
    void Stop();
    std::shared_ptr<boost::asio::io_context> &io_context_shared() 
    { return app_.io_context_shared(); }

protected:
    void DoAccept();
    void OnAccept(ConnectionPtr connection,  const boost::system::error_code &ec);
	Application &app_;

private:
    boost::asio::ip::tcp::acceptor acceptor_;
};

} // namespace net
} // namespace ipc
#endif // NET_TCPSERVER_H
