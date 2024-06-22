/*
 * @file connection.cpp
 * @author Songxi Yang
 * @mail ysx-cpp@gmail.com
 * @github https://github.com/ysx-cpp
 * @date Oct 08 2019
 */
#include "connection.h"
#include <iostream>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/functional/hash.hpp>
#include "connectionpool.h"
#include "heartbeat.h"

namespace ipc {
namespace net {

Connection::Connection(boost::asio::io_context &ioc) :
TcpHandler(ioc),
connction_pool_(nullptr),
heartbeat_(new Heartbeat(ioc))
{
}

Connection::Connection(boost::asio::io_context &ioc, ConnectionPool *connction_pool) :
TcpHandler(ioc),
connction_pool_(connction_pool),
heartbeat_(new Heartbeat(ioc))
{

}

void Connection::Start()
{
    if (connction_pool_)
    connction_pool_->AddConnection(ShaerdSelf());

    ReadSome();
}

void Connection::Stop()
{
	if (connction_pool_)
	{
		connction_pool_->RemoveConnection(ShaerdSelf());
	}
	else
	{
		Close();
	}
}

void Connection::StartHeartbeat()
{
	heartbeat_->StartTimer();
}

void Connection::SendHeartBeat()
{
	heartbeat_->Ping([&]() {
		Package pkg;
		pkg.set_cmd(0);
		SendData(pkg, "ping");
	});
}

void Connection::OnHeartbeat()
{
	if (heartbeat_->Stopped())
	{
		this->Stop();
	}
	else
	{
		heartbeat_->UpdateTimer();
	}
}

bool Connection::Connect(const std::string &host, unsigned short port)
{
	boost::system::error_code ec;
	boost::asio::ip::tcp::endpoint ep(boost::asio::ip::address::from_string(host), port);
	socket_.connect(ep, ec);
	return !ec;
}

void Connection::SendData(Package& pkg, const std::string &data)
{
	ByteArray array(data.begin(), data.end());
	SendData(pkg, array);
}

void Connection::SendData(Package& pkg, const ByteArray &data)
{
	pkg.set_seq(send_seq_);
	pkg.set_verify(GenerateVerify(data));
	pkg.Encode(data);
    WriteSome(pkg.data());

	std::string stringmsg1(data.begin(), data.end());
	LOGERR("ERROR verify1:" << GenerateVerify(data) << " data1:" << stringmsg1 << " size1:" << data.size());

	Package pkg2;
	pkg2.Decode(pkg.data());
	std::string stringmsg2(pkg2.data().begin(), pkg2.data().end());
	LOGERR("ERROR verify2:" << GenerateVerify(pkg2.data()) << " data2:" << stringmsg2 << " size2:" << pkg2.data().size());
}

std::shared_ptr<Connection> Connection::ShaerdSelf()
{
	return std::dynamic_pointer_cast<Connection>(shared_from_this());
}

void Connection::Complete(const ByteArrayPtr data)
{
	auto package = std::make_shared<Package>();
	package->Decode(*data);

	if (package->seq() != recv_seq_)
	{
		LOGERR("ERROR seq:" << package->seq());
		// return;
	}

	if (!CheckVerify(package->data(), package->verify()))
	{
		unsigned long long verify = GenerateVerify(package->data());
		LOGERR("ERROR verify:" << package->verify() << " data verify:" << verify << " data:" << std::string(package->data().begin(), package->data().end()));
		// return;
	}

	OnHeartbeat();

	++recv_seq_;
    if (connction_pool_)
        connction_pool_->OnReceveData(package, std::dynamic_pointer_cast<Connection>(shared_from_this()));
}

void Connection::Successfully(const std::size_t& write_bytes)
{
	++send_seq_;
	if (connction_pool_)
		connction_pool_->OnSendData(write_bytes);
}

bool Connection::CheckVerify(const ByteArray &data, uint64_t verify)
{
	return GenerateVerify(data) == verify;
}

uint64_t Connection::GenerateVerify(const ByteArray &data)
{
	return boost::hash_value<ByteArray>(data);
}

void Connection::Disconnect()
{
	heartbeat_->Stop();
	this->Stop();
}

} // namespace net
} // namespace ipc
