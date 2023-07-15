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

namespace ipc {
namespace net {

class ConnectionPool;
class Connection : public TcpHandler
{
public:
	explicit Connection(boost::asio::io_context &ioc);
	~Connection() = default;

	void Start();
    void Stop();
	void SetConnectionPool(ConnectionPool* conn_pool);
	bool Connect(const std::string& host, unsigned short port);
	void Send(Package& pkg);
	void EnableHeartbeat();
	int OnHeartbeat();

	bool connected() const { return connected_; }
	void set_connected(bool connected) { connected_ = connected; }
	
protected:
	std::shared_ptr<Connection> ShaerdSelf();
    void Complete(const ByteArrayPtr data) override;
	void Successfully(const std::size_t& write_bytes) override;
	bool CheckVerify(const ByteArray &data, uint64_t verify);
	uint64_t GenerateVerify(const ByteArray &data);
	void Disconnect() override;

private:
    ConnectionPool *connction_pool_;
	std::unique_ptr<Heartbeat> heartbeat_;
	bool connected_ = false;
	std::uint64_t send_seq_ = 0;
	std::uint64_t recv_seq_ = 0;
};
using ConnectionPtr = std::shared_ptr<Connection>;

} // namespace net
} // namespace ipc

#endif // NET_CONNECTION_H
