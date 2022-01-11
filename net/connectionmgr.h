/*
 * @file connectionmgr.h
 * @author Songxi Yang
 * @mail ysx-cpp@gmail.com
 * @github https://github.com/ysx-cpp
 * @date Oct 08 2019
 */
#ifndef NET_CONNECTION_MGR_H
#define NET_CONNECTION_MGR_H

#include <set>
#include <boost/noncopyable.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include "connection.h"

namespace fastlink {
namespace ipc {

class ConnectionMgr : private boost::noncopyable
{
    friend class Connection;
protected:
	virtual ~ConnectionMgr() = default;

    virtual int OnReceveData(const ByteArrayPtr data, ConnectionPtr connection) = 0;
	virtual int OnConnect(ConnectionPtr connection) = 0;
	virtual int OnDisconnect(ConnectionPtr connection) = 0;
	ConnectionPtr CreateConnection(boost::asio::io_context &ioc);
	ConnectionPtr CreateConnectionSsl(boost::asio::io_context &ioc);
	void AddConnection(ConnectionPtr connection);
	void RemoveConnection(ConnectionPtr connection);
	void RemoveAllConnection();
    ConnectionPtr GetConnection() const;
	ConnectionPtr GetConnection(const ConnectionPtr &connection) const;

    std::set<ConnectionPtr> &GetAllConnection() {return connection_pool_;}
    size_t Index() const;

private:
	boost::recursive_mutex mutex_;                      /* 全局锁 */
	std::set<ConnectionPtr> connection_pool_;           /* 连接池 */
};
using ConnectionMgrPtr = boost::shared_ptr<ConnectionMgr>;

} // namespace ipc
} // namespace fastlink
#endif // NET_CONNECTION_MGR_H
