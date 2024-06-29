/*
 * @file connectionpool.cpp
 * @author Songxi Yang
 * @mail ysx-cpp@gmail.com
 * @github https://github.com/ysx-cpp
 * @date Oct 08 2019
 */
#include "connectionpool.h"
#include <algorithm>
#include <boost/make_shared.hpp>
#ifndef WIN32
#include <boost/asio/ssl/context_base.hpp>
#include <boost/asio/ssl/context.hpp>
#include <boost/asio/ssl/stream.hpp>
#endif

namespace ipc {
namespace net {

using namespace boost::asio;

ConnectionPtr ConnectionPool::CreateConnection(boost::asio::io_context &ioc, ConnectionPool *connection_pool)
{
	return std::make_shared<Connection>(ioc, connection_pool);
}

ConnectionPtr ConnectionPool::CreateConnectionSSL(boost::asio::io_context &ioc, ConnectionPool *connection_pool)
{
#if 0
	ssl::context ctx(ssl::context::sslv23);
	boost::system::error_code error;
	ctx.set_options(boost::asio::ssl::context::default_workarounds |
		boost::asio::ssl::context::no_sslv2 |
		boost::asio::ssl::context::single_dh_use, error);

	//RPCServerPtr ptrServer = boost::dynamic_pointer_cast<RPCServer>(shared_from_this());
	//ctx.set_password_callback(boost::bind(&RPCServer::getPassword, ptrServer));

	ctx.use_certificate_chain_file("~/.ssh/res.pubile");
	ctx.use_private_key_file("~/.ssh/res.private", boost::asio::ssl::context::pem);
	ctx.use_tmp_dh_file("~/.ssh/tmp_dh_file");

	ssl::stream<ip::tcp::socket> sock(ioc, ctx);
	return boost::make_shared<Connection>(std::move(sock));
#else
	return CreateConnection(ioc, connection_pool);
#endif
}

void ConnectionPool::AddConnection(ConnectionPtr connection)
{
	boost::recursive_mutex::scoped_lock rlock(mutex_);
	connection_pool_.insert(connection);
	this->OnConnect(connection);
}

void ConnectionPool::RemoveConnection(ConnectionPtr connection)
{
	boost::recursive_mutex::scoped_lock rlock(mutex_);
	if (connection_pool_.find(connection) != connection_pool_.end())
	{
		connection_pool_.erase(connection);
	}
}

void ConnectionPool::RemoveAllConnection()
{
	boost::recursive_mutex::scoped_lock rlock(mutex_);
	connection_pool_.clear();
}

ConnectionPtr ConnectionPool::GetConnection() const
{
    auto it = connection_pool_.cbegin();
    it = std::next(it, Index());
	assert(it != connection_pool_.cend());
    return *it;
}

ConnectionPtr ConnectionPool::GetConnection(const ConnectionPtr &connection) const
{
	auto it = connection_pool_.find(connection);
	if (it == connection_pool_.end())
		return nullptr;

	return *it;
}

ConnectionPtr ConnectionPool::GetConnection(size_t index) const
{
	const auto size = connection_pool_.size();
	auto it = connection_pool_.cbegin();
	it = std::next(it, index % size);
	assert(it != connection_pool_.cend());
	return *it;
}

size_t ConnectionPool::Index() const
{
	static unsigned index = 0;
	assert(!connection_pool_.empty());
	return index++ % connection_pool_.size();
}

} // namespace net
} // namespace ipc
