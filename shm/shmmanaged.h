/*
 * @file shmcontainer.h
 * @author Songxi Yang
 * @mail ysx-cpp@gmail.com
 * @github https://github.com/ysx-cpp
 * @date Oct 08 2021
 */
#ifndef ICP_SHM_MANAGED_H
#define ICP_SHM_MANAGED_H
#include <iostream>
#include <boost/noncopyable.hpp>
#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/thread/detail/singleton.hpp>

namespace ipc {
namespace shm {

using boost::interprocess::managed_shared_memory;
using SegmentManager = managed_shared_memory::segment_manager;

template <typename T, typename SegmentManager = SegmentManager>
using ShmAllocator = boost::interprocess::allocator<T, SegmentManager>;

class ShmManaged : private boost::noncopyable
{
public:
	bool Initialize(const std::string &key, size_t size)
	{
		// Create a managed shared memory
		shm_key_ = key;
		// shm_managed_ = std::make_unique<managed_shared_memory>(boost::interprocess::open_or_create, shm_key_.c_str(), size);
		shm_managed_.reset(new managed_shared_memory(boost::interprocess::open_or_create, shm_key_.c_str(), size));
		assert(shm_managed_ != nullptr);

		// Create a allocator shared memory
		// shm_allocator_ = std::make_unique<ShmAllocator<void>>(shm_managed_->get_segment_manager());
		shm_allocator_.reset(new ShmAllocator<void>(shm_managed_->get_segment_manager()));
		assert(shm_allocator_ != nullptr);

		return shm_managed_->get_size() == size;
	}

	managed_shared_memory &GetShmManaged()
	{
		return *shm_managed_;
	}

	ShmAllocator<void> &GetShmAllocator()
	{
		return *shm_allocator_;
	}

	void Grow()
	{
		try
		{
			// 1.增长大小：managed_shared_memory::grow 的增长大小必须大于 0，并且通常应大于当前剩余的共享内存大小。
			// 2.跨进程协调：如果多个进程使用同一个共享内存段，必须协调增长操作，确保所有进程都知道共享内存已增长。
			// 3.内存布局：增长操作可能影响共享内存中已有数据结构的布局，必须小心处理内存引用和指针。
			//Now that the segment is not mapped grow it adding extra ${size} bytes
			shm_managed_->grow(shm_key_.c_str(), shm_managed_->get_size());
			shm_managed_.reset(new managed_shared_memory(boost::interprocess::open_only, shm_key_.c_str()));
		}
		catch (...)
		{
			std::cerr << "grow error!" << std::endl;
		}
	}

	void ShrinkToFit()
	{
		try
		{
			//Now minimize the size of the segment
			shm_managed_->shrink_to_fit(shm_key_.c_str());
		}
		catch (...)
		{
			std::cerr << "shrink_to_fit error!" << std::endl;
		}
	}

private:
	std::string shm_key_;
    std::unique_ptr<managed_shared_memory> shm_managed_;
	std::unique_ptr<ShmAllocator<void>> shm_allocator_;
};
using ShmManagedSingle = boost::detail::thread::singleton<ShmManaged>;

} //namespace shm
} //namespace ipc 

#endif //ICP_SHM_MANAGED_H