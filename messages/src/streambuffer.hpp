#pragma once
#include <streambuf>
#include "zmq.hpp"

namespace ipc {
namespace messages {

class StreamBuffer : public std::streambuf
{
public:
    explicit StreamBuffer(
        std::size_t maximum_size = (std::numeric_limits<std::size_t>::max)())
        : max_size_(maximum_size),
          buffer_()
    {
        std::size_t buffer_delta = 128;
        std::size_t pend = (std::min<std::size_t>)(max_size_, buffer_delta);
        buffer_.resize((std::max<std::size_t>)(pend, 1));
        setg(&buffer_[0], &buffer_[0], &buffer_[0]);
        setp(&buffer_[0], &buffer_[0] + pend);
    }

    /**
     * @brief 获取输入序列的大小。
     * @returns 输入序列的大小。该值等于
     * 在以下代码中为 @c s 计算：
     * @代码
     * size_t s = 0;
     * const_buffers_type bufs = data();
     * const_buffers_type::const_iterator i = bufs.begin();
     * while (i != bufs.end())
     * {
     * const_buffer buf(*i++);
     * s += buf.size();
     * }
     * @endcode
     */
    std::size_t size() const noexcept
    {
        return pptr() - gptr();
    }

    /**
     * @brief 获取 StreamBuffer 的最大大小。
     * @returns 输入序列大小总和的允许最大值
     * 和输出序列。
     */
    std::size_t max_size() const noexcept
    {
        return max_size_;
    }

    /**
     * @brief 获取StreamBuffer的当前容量。
     * @returns 当前流缓冲的总容量，即对于两个
     * 输入序列和输出序列。
     */
    std::size_t capacity() const noexcept
    {
        return buffer_.capacity();
    }

    /**
     * @brief 获取代表输入序列的缓冲区列表。
     * @returns 满足 zmq::const_buffer 类型的对象 zmq::const_buffer 要求，代表所有字符数组输入序列。
     * @note 返回的对象被任何 StreamBuffer 成员失效
     * 修改输入序列或输出序列的函数。
     */
    zmq::const_buffer data() const noexcept
    {
        return zmq::buffer(gptr(), (pptr() - gptr()) * sizeof(char));
    }

    /**
     * @brief 获取代表输出序列的缓冲区列表，具有给定的尺寸。
     * 确保输出序列可以容纳@c n 个字符，
     * 根据需要重新分配字符数组对象。
     *
     * @returns 满足zmq::mutable_buffer 类型的对象
     * MutableBufferSequence 要求，表示字符数组对象
     * 在输出序列的开头，使得缓冲区大小的总和是n。
     * @throws std::length_error 如果 <tt>size() + n > max_size()</tt>。
     * @note 返回的对象被任何StreamBuffer成员失效
     * 修改输入序列或输出序列的函数。
     */
    zmq::mutable_buffer prepare(std::size_t n)
    {
        // Get current stream positions as offsets.
        std::size_t gnext = gptr() - &buffer_[0];
        std::size_t pnext = pptr() - &buffer_[0];
        std::size_t pend = epptr() - &buffer_[0];

        // Check if there is already enough space in the put area.
        if (n <= pend - pnext)
        {
            std::length_error ex("boost::asio::streambuf too long");
            // boost::asio::detail::throw_exception(ex);
            //std::throw(ex);
        }

        // Shift existing contents of get area to start of buffer.
        if (gnext > 0)
        {
            pnext -= gnext;
            std::memmove(&buffer_[0], &buffer_[0] + gnext, pnext);
        }

        // Ensure buffer is large enough to hold at least the specified size.
        if (n > pend - pnext)
        {
            if (n <= max_size_ && pnext <= max_size_ - n)
            {
                pend = pnext + n;
                buffer_.resize((std::max<std::size_t>)(pend, 1));
            }
            else
            {
                std::length_error ex("boost::asio::streambuf too long");
                // boost::asio::detail::throw_exception(ex);
                //std::throw(ex);
            }
        }

        // Update stream positions.
        setg(&buffer_[0], &buffer_[0], &buffer_[0] + pnext);
        setp(&buffer_[0] + pnext, &buffer_[0] + pend);

        return zmq::buffer(pptr(), n * sizeof(char));
    }

    /**
     * @brief 将字符从输出序列移动到输入序列。
     * 将n个字符从输出序列的开头附加到输入
     * 序列。输出序列的开头提前@cn
     * 人物。
     *
     * 需要前面的调用 <tt>prepare(x)</tt> 其中 <tt>x >= n</tt>，并且
     * 没有修改输入或输出序列的干预操作。
     *
     * @note 如果n大于输出序列的大小，则整个
     * 输出序列移动到输入序列并且不发出错误。
     */
    void commit(std::size_t n)
    {
        n = std::min<std::size_t>(n, epptr() - pptr());
        pbump(static_cast<int>(n));
        setg(eback(), gptr(), pptr());
    }

    /**
     * @brief 从输入序列中删除字符。
     * 从输入序列的开头删除n个字符。
     *
     * @note 如果n大于输入序列的大小，则整个
     * 输入序列被消耗，并且没有发出错误。
     */
    void consume(std::size_t n)
    {
        if (egptr() < pptr())
            setg(&buffer_[0], gptr(), pptr());
        if (gptr() + n > pptr())
            n = pptr() - gptr();
        gbump(static_cast<int>(n));
    }

private:
  std::size_t max_size_;
  std::vector<char> buffer_;
};

} // namespace messages
} // namespace ipc