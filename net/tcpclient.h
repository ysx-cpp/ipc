/*
 * @file tcpclient.h
 * @author Songxi Yang
 * @mail ysx-cpp@gmail.com
 * @github https://github.com/ysx-cpp
 * @date Oct 08 2019
 */
#ifndef NET_TCPCLIENT_H
#define NET_TCPCLIENT_H

#include "package.h"
#include "tcphandler.h"
#include "connection.h"

namespace fastlink {
namespace ipc {

struct ConnectOption
{
    std::string host;
    uint16_t port;
    int timeout_ms;
};

class TcpClient : public Connection
{
public:
    TcpClient(boost::asio::io_context &ioc);
    void CreateConnect(const std::string &host, uint16_t port, int timeout = 200);
    void Ping();
    void SendData(const std::string &msg);
    void Disconnect() override;

private:
	int expire() const { return 3; }
    boost::asio::io_context &ioc_;
    boost::asio::deadline_timer timer_;
    boost::asio::deadline_timer timer_conn_;
    ConnectOption option_;
};

} // namespace ipc
} // namespace fastlink

#endif // NET_TCPCLIENT_H
