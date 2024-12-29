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
#include "connection.h"

namespace ipc {
namespace net {

struct ConnectOption
{
    std::string host;
    uint16_t port;
    int timeout_ms;
};

class TcpClient : public Connection
{
public:
    explicit TcpClient(boost::asio::io_context &ioc);
    void CreateConnect(const std::string &host, uint16_t port, int timeout = 200);
    void SendPackage(PackagePtr package, const std::string &msg);

private:
    boost::asio::io_context &ioc_;
    boost::asio::deadline_timer timer_;
    ConnectOption option_;
};

} // namespace net
} // namespace ipc

#endif // NET_TCPCLIENT_H
