/*
 * @file package.cpp
 * @author Songxi Yang
 * @mail ysx-cpp@gmail.com
 * @github https://github.com/ysx-cpp
 * @date Oct 08 2019
 */
#include <iostream>
#include <boost/asio/streambuf.hpp>
#include "package.h"

namespace fastlink {
namespace ipc {

using namespace boost::asio;

void Head::Encode(ByteArray &data)
{
	this->head_size = static_cast<uint16_t>(sizeof(Head));
	this->data_size = static_cast<uint16_t>(data.size());
	auto phead = reinterpret_cast<const unsigned char *>(this);
	data.insert(data.begin(), phead, phead + this->head_size);
}

void Head::Decode(ByteArray &data)
{
	if (data.size() >= sizeof(Head))
		data.assign(data.begin() + this->head_size, data.end());
}

void Head::Decode(const ByteArray &data, std::string &msg)
{
	if (data.size() >= sizeof(Head))
		msg.assign(data.begin() + this->head_size, data.end());
}

void Package::Encode()
{
    const unsigned char *phead	= reinterpret_cast<unsigned char*>(&head_);
    data_.insert(data_.begin(), phead, phead + head_.head_size);
}

void Package::Encode(const std::string &data)
{
    ByteArray array(data.begin(), data.end());
    Encode(array);
}

void Package::Encode(const ByteArray &data)
{
    size_t head_size = sizeof(PackageHead);
    size_t data_size = static_cast<uint16_t>(data.size());
	
    data_.resize(head_size + data_size);
    PackageHead *phead   = reinterpret_cast<PackageHead*>(&data_[0]);
    phead->head_size	 = head_size;
    phead->data_size     = data_size;
    phead->uid           = this->uid();
    phead->cmd           = this->cmd();
    phead->src           = this->src();
    phead->dst          = this->dst();

    data_.insert(data_.begin() + phead->head_size, data.begin(), data.end());
}

void Package::Decode(const std::string &data)
{
    ByteArray array(data.begin(), data.end());
    Decode(array);
}

void Package::Decode(const ByteArray &data)
{
    const PackageHead *phead    = reinterpret_cast<const PackageHead*>(&data[0]);
    this->set_head_size(phead->head_size);
    this->set_data_size(phead->data_size);
    this->set_uid(phead->uid);
    this->set_cmd(phead->cmd);
    this->set_src(phead->src);
    this->set_dst(phead->dst);

    data_.assign(data.begin() + phead->head_size, data.end());
}

} // namespace ipc
} // namespace fastlink
