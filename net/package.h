/*
 * @file package.h
 * @author Songxi Yang
 * @mail ysx-cpp@gmail.com
 * @github https://github.com/ysx-cpp
 * @date Oct 08 2019
 */
#ifndef NET_PACKAGE_H
#define NET_PACKAGE_H

#include <string>
#include <boost/asio.hpp>
#include <boost/make_shared.hpp>
#include <boost/enable_shared_from_this.hpp>

namespace ipc {
namespace net {

using Byte = uint8_t;
using ByteArray = std::vector<Byte>;
using ByteArrayPtr = std::shared_ptr<ByteArray>;

struct Head
{
    std::uint16_t  head_size      = 0;
    std::uint16_t data_size      = 0;
	void Encode(ByteArray &data);
    void Decode(const ByteArray &data, std::string &msg);
};

struct PackageHead
{
    std::uint16_t head_size      = sizeof(PackageHead);
    std::uint16_t data_size      = 0;
    std::uint64_t uid            = 0;
    std::uint16_t cmd            = 0;
    std::uint32_t src            = 0; //from server ID
    std::uint32_t dst            = 0; //destination is server ID
	std::uint64_t seq            = 0; //sequence
	std::uint64_t verify         = 0; //verify
};

class Package : public boost::enable_shared_from_this<Package>
{
public:
	///将包内的数据加上包头（用于转发）
	void Encode();
    ///将传入的数据加上包头
    void Encode(const std::string &data);
    void Encode(const ByteArray &data);

	///将包头和数据分开
    void Decode(const std::string &data);
    void Decode(const ByteArray &data);

	///填充数据
	void FullData(const std::string &data);
	void FullData(const ByteArray &data);

	const ByteArray &data() const { return data_; }
    const unsigned char *pdata() const { return &(data_[0]);}
    bool IsFull() const {return data_.size() >= data_size();}
    bool IsEmpty() const {return data_.empty();}

    void set_head_size(std::uint16_t head_size) {head_.head_size = head_size;}
    void set_data_size(std::uint16_t data_size) {head_.data_size = data_size;}
    void set_uid(std::uint64_t uid) {head_.uid = uid;}
    void set_cmd(std::uint16_t cmd) {head_.cmd = cmd;}
    void set_src(std::uint32_t src) {head_.src = src;}
	void set_dst(std::uint32_t dst) { head_.dst = dst; }
	void set_seq(std::uint64_t seq) { head_.seq = seq; }
	void set_verify(std::uint64_t verify) { head_.verify = verify; }

    std::uint16_t head_size() const {return head_.head_size;}
    std::uint16_t data_size() const {return head_.data_size;}
    std::uint64_t uid() const {return head_.uid;}
    std::uint16_t cmd() const {return head_.cmd;}
    std::uint32_t src() const {return head_.src;}
	std::uint32_t dst() const { return head_.dst; }
	std::uint64_t seq() const { return head_.seq; }
	std::uint64_t verify() const { return head_.verify; }

public:
    PackageHead head_;
    ByteArray data_;
};

using PackagePtr = std::shared_ptr<Package>;

} // namespace net
} // namespace ipc

#endif // NET_PACKAGE_H
