/*
 * @file connectionmgr.cpp
 * @author Songxi Yang
 * @mail ysx-cpp@gmail.com
 * @github https://github.com/ysx-cpp
 * @date Oct 08 2019
 */
#include "connectionmgr.h"
#include <algorithm>
#include <boost/make_shared.hpp>
#ifndef WIN32
#include <boost/asio/ssl/context_base.hpp>
#include <boost/asio/ssl/context.hpp>
#include <boost/asio/ssl/stream.hpp>
#endif

namespace fastlink {
namespace net {

using namespace boost::asio;

int ConnectionMgr::OnReceveData(const ByteArrayPtr, ConnectionPtr)
{
    return 0;
}

int ConnectionMgr::OnConnect(ConnectionPtr /*connection*/)
{
	return 0;
}

int ConnectionMgr::OnDisconnect(ConnectionPtr /*connection*/)
{
	return 0;
}

ConnectionPtr ConnectionMgr::CreateConnection(boost::asio::io_context &ioc)
{
	return std::make_shared<Connection>(ioc);
}

ConnectionPtr ConnectionMgr::CreateConnectionSsl(boost::asio::io_context &ioc)
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
	return CreateConnection(ioc);
#endif
}

void ConnectionMgr::AddConnection(ConnectionPtr connection)
{
	boost::recursive_mutex::scoped_lock rlock(mutex_);
	connection_pool_.insert(connection);
	this->OnConnect(connection);
}

void ConnectionMgr::RemoveConnection(ConnectionPtr connection)
{
	boost::recursive_mutex::scoped_lock rlock(mutex_);
	if (connection_pool_.find(connection) != connection_pool_.end())
	{
		this->OnDisconnect(connection);
		connection_pool_.erase(connection);
	}
}

void ConnectionMgr::RemoveAllConnection()
{
	boost::recursive_mutex::scoped_lock rlock(mutex_);
	connection_pool_.clear();
}

ConnectionPtr ConnectionMgr::GetConnection() const
{
    auto it = connection_pool_.cbegin();
    std::next(it, Index());
    return *it;
}

ConnectionPtr ConnectionMgr::GetConnection(const ConnectionPtr &connection) const
{
	auto it = connection_pool_.lower_bound(connection);
	if (it == connection_pool_.end())
		return *connection_pool_.begin();

	return *it;
}

size_t ConnectionMgr::Index() const
{
	static unsigned index = 0;
	assert(!connection_pool_.empty());
	return index++ % connection_pool_.size();
}

} // namespace net
} // namespace fastlink
