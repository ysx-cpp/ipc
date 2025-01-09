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
#include "package.h"
#include "logdefine.h"

namespace ipc {
namespace net {

Connection::Connection(boost::asio::io_context &ioc) :
TcpHandler(ioc),
connction_pool_(nullptr),
heartbeat_(new Heartbeat(ioc)),
send_seq_(0),
recv_seq_(0)
{
}

Connection::Connection(boost::asio::io_context &ioc, ConnectionPool *connction_pool) :
TcpHandler(ioc),
connction_pool_(connction_pool),
heartbeat_(new Heartbeat(ioc)),
send_seq_(0),
recv_seq_(0)
{
}

void Connection::Start()
{
    if (connction_pool_)
    connction_pool_->AddConnection(ShaerdSelf());

	StartHeartbeat();
	Read();
}

void Connection::Stop()
{
	heartbeat_->Stop();
	impl_.Close();
	if (connction_pool_)
	{
		connction_pool_->RemoveConnection(ShaerdSelf());
	}
}

bool Connection::Connect(const std::string &host, unsigned short port)
{
	boost::system::error_code ec;
	boost::asio::ip::tcp::endpoint ep(boost::asio::ip::address::from_string(host), port);
	socket_.connect(ep, ec);
	return !ec;
}

void Connection::SendData(Package& pkg, const ByteArray &data)
{
	std::string array(data.begin(), data.end());
	SendData(pkg, array);
}

void Connection::SendData(Package& pkg, const std::string &data)
{
    switch (pkg.cmd())
	{
    case NET_HEARTBEAT:
    case NET_PACKAGE_REPLY:
		break;
	default:
		pkg.set_verify(GenerateVerify(data));
		break;
	}

	pkg.set_seq(send_seq_);
	pkg.set_data_size(data.size());
	pkg.Encode(data);
    WriteSome(pkg.data());
}

void Connection::Complete(const std::string &data)
{
	auto package = std::make_shared<Package>();
	package->Decode(data);

	std::string stringmsg(package->data().begin(), package->data().end());
	NET_LOGINFO("INFO verify1:" << package->verify() << " cmd:" << package->cmd() << " data:" << stringmsg << " size:" << package->data().size());

    switch (package->cmd())
	{
    case NET_HEARTBEAT:
		OnHeartbeat();
		IncrRecvSeq();
		return;
    case NET_PACKAGE_REPLY:
		IncrSendSeq(package);
		return;
	default:
		IncrRecvSeq();
		break;
	}

	if (package->seq() + 1 != recv_seq_)
	{
		NET_LOGERR("ERROR send_seq:" << package->seq() + 1 << " rev_sqe:" << recv_seq_);
		return;
	}

	unsigned long long verify = GenerateVerify(package->data());
	if (package->verify() != verify)
	{
		NET_LOGERR("ERROR verify:" << package->verify() << " data verify:" << verify << " data:" << std::string(package->data().begin(), package->data().end()));
		return;
	}

    if (connction_pool_)
        connction_pool_->OnReceveData(package, ShaerdSelf());
	else
		OnReceveData(package);
}

void Connection::StartHeartbeat()
{
	if (connction_pool_)
	{
		heartbeat_->StartTimer();
	}

	heartbeat_->Ping(ShaerdSelf());
}

void Connection::OnHeartbeat()
{
	if (!Connected())
		return;

	if (heartbeat_->Stopped())
	{
		impl_.Close();
	}
	else if (connction_pool_)
	{
		heartbeat_->UpdateTimer();
	}
}

std::shared_ptr<Connection> Connection::ShaerdSelf()
{
	return std::dynamic_pointer_cast<Connection>(shared_from_this());
}

void Connection::Successfully(const std::size_t& write_bytes)
{
	if (connction_pool_)
		connction_pool_->OnSendData(write_bytes, ShaerdSelf());
	else
		OnSendData(write_bytes);
}

void Connection::Shutdown()
{
	if (connction_pool_)
	{
		connction_pool_->OnDisconnect(ShaerdSelf());
	}
	else
	{
		OnDisconnect();
	}
}

void Connection::IncrRecvSeq()
{
	++recv_seq_;
	Package pkg;
    pkg.set_cmd(static_cast<uint16_t>(NET_PACKAGE_REPLY));
	pkg.set_seq(recv_seq_);
	SendData(pkg, "reply");
	NET_LOGERR("INFO pkg.req:" << pkg.seq() << " send_seq:" << send_seq_ << " rev_seq:" << recv_seq_);
}

void Connection::IncrSendSeq(const PackagePtr package)
{
	++send_seq_;
	NET_LOGERR("INFO pkg.seq:" << package->seq() << " send_seq:" << send_seq_ << " rev_seq:" << recv_seq_);
}

uint64_t Connection::GenerateVerify(const std::string &data) const
{
    std::hash<std::string> hash_str_fn;
    return hash_str_fn(data);
}

uint64_t Connection::GenerateVerify(const ByteArray &data) const
{
	std::string str(data.begin(), data.end());
	return GenerateVerify(str);
}

} // namespace net
} // namespace ipc
