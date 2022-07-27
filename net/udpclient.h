/*
 * @file udphandler.h
 * @author Songxi Yang
 * @mail ysx-cpp@gmail.com
 * @github https://github.com/ysx-cpp
 * @date 13 Apr 2024
 */
#ifndef NET_UDP_CLINET_H
#define NET_UDP_CLINET_H

#include "udphandler.h"

namespace ipc {
namespace net {

class UdpClient : public UdpHandler
{
public:
    UdpClient(boost::asio::io_context &ioc, UdpEndpoint &&endpoint);

    
protected:
    void Complete(const ByteArrayPtr data, const UdpEndpoint &endpoint) override;
	void Disconnect() override;
};

} // namespace net
} // namespace ipc
#endif // NET_UDP_CLINET_H