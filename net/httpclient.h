/*
 * @file tcphandler.h
 * @author Songxi Yang
 * @mail ysx-cpp@gmail.com
 * @github https://github.com/ysx-cpp
 * @date Oct 08 2019
 */
#ifndef NET_HTTP_CLIENT_H
#define NET_HTTP_CLIENT_H

#include <boost/asio.hpp>

namespace ipc {
namespace net {

class HttpClient {
public:
    HttpClient(const std::string& server, const std::string& path);

    void Run();
    void SendRequest();

private:
    boost::asio::io_context io_context_;
    boost::asio::ip::tcp::resolver resolver_;
    boost::asio::ip::tcp::socket socket_;
    std::string request_;
};

} // namespace net
} // namespace ipc
#endif // NET_HTTP_CLIENT_H