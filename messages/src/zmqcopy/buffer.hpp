#pragma once
#include <streambuf>
#include "zmq.hpp"

namespace ipc {
namespace messages {

class StringBuffer : public std::streambuf
{
public:
    StringBuffer()
    {
    }

    /// Get the size of the input sequence.
    /**
     * @returns The size of the input sequence. The value is equal to that
     * calculated for @c s in the following code:
     * @code
     * size_t s = 0;
     * const_buffers_type bufs = data();
     * const_buffers_type::const_iterator i = bufs.begin();
     * while (i != bufs.end())
     * {
     *   const_buffer buf(*i++);
     *   s += buf.size();
     * }
     * @endcode
     */
    std::size_t size() const noexcept
    {
        return pptr() - gptr();
    }

    /// Get the maximum size of the basic_streambuf.
    /**
     * @returns The allowed maximum of the sum of the sizes of the input sequence
     * and output sequence.
     */
    std::size_t max_size() const noexcept
    {
        return max_size_;
    }

    /// Get the current capacity of the basic_streambuf.
    /**
     * @returns The current total capacity of the streambuf, i.e. for both the
     * input sequence and output sequence.
     */
    std::size_t capacity() const noexcept
    {
        return buffer_.capacity();
    }

    /// Get a list of buffers that represents the output sequence, with the given
    /// size.
    /**
     * Ensures that the output sequence can accommodate @c n characters,
     * reallocating character array objects as necessary.
     *
     * @returns An object of type @c mutable_buffers_type that satisfies
     * MutableBufferSequence requirements, representing character array objects
     * at the start of the output sequence such that the sum of the buffer sizes
     * is @c n.
     *
     * @throws std::length_error If <tt>size() + n > max_size()</tt>.
     *
     * @note The returned object is invalidated by any @c basic_streambuf member
     * function that modifies the input sequence or output sequence.
     */
    mutable_buffer Prepare(std::size_t n)
    {
        // Get current stream positions as offsets.
        std::size_t gnext = gptr() - &buffer_[0];
        std::size_t pnext = pptr() - &buffer_[0];
        std::size_t pend = epptr() - &buffer_[0];

        // Check if there is already enough space in the put area.
        if (n <= pend - pnext)
        {
            return;
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
                std::throw(ex);
            }
        }

        // Update stream positions.
        setg(&buffer_[0], &buffer_[0], &buffer_[0] + pnext);
        setp(&buffer_[0] + pnext, &buffer_[0] + pend);

        return mutable_buffer(buffer_->pptr(), n * sizeof(char));
    }

    /// Move characters from the output sequence to the input sequence.
    /**
     * Appends @c n characters from the start of the output sequence to the input
     * sequence. The beginning of the output sequence is advanced by @c n
     * characters.
     *
     * Requires a preceding call <tt>prepare(x)</tt> where <tt>x >= n</tt>, and
     * no intervening operations that modify the input or output sequence.
     *
     * @note If @c n is greater than the size of the output sequence, the entire
     * output sequence is moved to the input sequence and no error is issued.
     */
    void Commit(std::size_t n)
    {
        n = std::min<std::size_t>(n, epptr() - pptr());
        pbump(static_cast<int>(n));
        setg(eback(), gptr(), pptr());
    }

    /// Remove characters from the input sequence.
    /**
     * Removes @c n characters from the beginning of the input sequence.
     *
     * @note If @c n is greater than the size of the input sequence, the entire
     * input sequence is consumed and no error is issued.
     */
    void Consume(std::size_t n)
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
}

} // namespace messages
} // namespace ipc