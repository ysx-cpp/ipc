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

namespace ipc {
namespace net {

static const int	NET_HEARTBEAT = 0;			// 心跳

class Heartbeat;
class ConnectionPool;
class Connection : public TcpHandler
{
public:
	explicit Connection(boost::asio::io_context &ioc);
	explicit Connection(boost::asio::io_context &ioc, ConnectionPool *connction_pool);
	~Connection() override = default;

	void Start();
    void Stop();
	bool Connect(const std::string& host, unsigned short port);
	void SendData(Package& pkg, const ByteArray &data);
	void SendData(Package& pkg, const std::string &data);
	bool Connected() const {return impl_.Connected();}

protected:
	virtual int OnReceveData(const PackagePtr package) {return 0;}
	virtual void OnSendData(const std::size_t& write_bytes) {}
	virtual int OnConnect() {return 0;}
	virtual int OnDisconnect() {return 0;}

protected:
	std::shared_ptr<Connection> ShaerdSelf();
    ConnectionPool *connction_pool_;

private:
	void StartHeartbeat();
	void OnHeartbeat();

private:
    void Complete(const std::string &data) override final;
	void Successfully(const std::size_t& write_bytes) override  final;
	void Shutdown() override final;

private:
	void IncrRecvSeq(const PackagePtr &package);
	void IncrSendSeq(const Package &pkg);
	uint64_t GenerateVerify(const std::string &data) const;
	uint64_t GenerateVerify(const ByteArray &data) const;

private:
	std::shared_ptr<Heartbeat> heartbeat_;
	std::atomic<uint64_t> send_seq_;
	std::atomic<uint64_t> recv_seq_;
};
using ConnectionPtr = std::shared_ptr<Connection>;

} // namespace net
} // namespace ipc

#endif // NET_CONNECTION_H
