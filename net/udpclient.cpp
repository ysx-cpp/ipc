#include "udpclient.h"

namespace ipc {
namespace net {

UdpClient::UdpClient(boost::asio::io_context &ioc, UdpEndpoint &&endpoint) :
    UdpHandler(ioc, std::move(endpoint))
{

}

void UdpClient::Complete(const ByteArrayPtr data, const UdpEndpoint &endpoint)
{

}

void UdpClient::Disconnect()
{

}

} // namespace net
} // namespace ipc