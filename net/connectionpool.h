/*
 * @file connectionpool.h
 * @author Songxi Yang
 * @mail ysx-cpp@gmail.com
 * @github https://github.com/ysx-cpp
 * @date Oct 08 2019
 */
#ifndef NET_CONNECTION_POOL_H
#define NET_CONNECTION_POOL_H

#include <set>
#include <boost/noncopyable.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include "connection.h"

namespace ipc {
namespace net {

class ConnectionPool : private boost::noncopyable
{
    friend class Connection;
protected:
	ConnectionPool() = default;
	virtual ~ConnectionPool() = default;

    virtual int OnReceveData(const PackagePtr package, ConnectionPtr connection) = 0;
	virtual void OnSendData(const std::size_t& write_bytes, ConnectionPtr connection) = 0;
	virtual int OnConnect(ConnectionPtr connection) = 0;
	virtual int OnDisconnect(ConnectionPtr connection) = 0;

	ConnectionPtr CreateConnection(boost::asio::io_context &ioc, ConnectionPool *connection_pool);
	ConnectionPtr CreateConnectionSSL(boost::asio::io_context &ioc, ConnectionPool *connection_pool);
	void AddConnection(ConnectionPtr connection);
	void RemoveConnection(ConnectionPtr connection);
	void RemoveAllConnection();
    ConnectionPtr GetConnection() const;
	ConnectionPtr GetConnection(const ConnectionPtr &connection) const;
	ConnectionPtr GetConnection(size_t index) const;

    std::set<ConnectionPtr> &GetAllConnection() {return connection_pool_;}
    size_t Index() const;

private:
	boost::recursive_mutex mutex_;                      /* 全局锁 */
	std::set<ConnectionPtr> connection_pool_;           /* 连接池 */
};
using ConnectionPoolPtr = boost::shared_ptr<ConnectionPool>;

} // namespace net
} // namespace ipc
#endif // NET_CONNECTION_POOL_H
