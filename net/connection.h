/*
 * @file connection.h
 * @author Songxi Yang
 * @mail ysx-cpp@gmail.com
 * @github https://github.com/ysx-cpp
 * @date Oct 08 2019
 */
#ifndef NET_CONNECTION_H
#define NET_CONNECTION_H

#include <memory>
#include "tcphandler.h"
#include "heartbeat.h"

namespace fastlink {
namespace net {

class ConnectionMgr;
class Connection : public TcpHandler
{
public:
	explicit Connection(boost::asio::io_context &ioc);
	~Connection() = default;

	void Start();
    void Stop();
	void SetManager(ConnectionMgr *manager);
	bool Connect(const std::string &host, unsigned short port);
	void Send(ByteArray &data);
	void EnableHeartbeat();
	int OnHeartbeat();

	bool connected() const { return connected_; }
	void set_connected(bool connected) { connected_ = connected; }
	
protected:
	std::shared_ptr<Connection> ShaerdSelf();
    void Complete(const ByteArrayPtr data) override;
	void Disconnect() override;

private:
    ConnectionMgr *manager_;
	std::unique_ptr<Heartbeat> heartbeat_;
	bool connected_ = false;
};
using ConnectionPtr = std::shared_ptr<Connection>;

} // namespace net
} // namespace fastlink

#endif // NET_CONNECTION_H
