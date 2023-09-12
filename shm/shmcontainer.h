/*
 * @file shmcontainer.h
 * @author Songxi Yang
 * @mail ysx-cpp@gmail.com
 * @github https://github.com/ysx-cpp
 * @date Oct 08 2021
 */
#ifndef ICP_SHM_CONTAINER_H
#define ICP_SHM_CONTAINER_H
#include <boost/noncopyable.hpp>
#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/interprocess/containers/map.hpp>
#include <boost/interprocess/containers/vector.hpp>
#include <boost/interprocess/containers/list.hpp>
#include <boost/interprocess/containers/string.hpp>
#include <boost/thread/detail/singleton.hpp>

namespace ipc {
namespace shm {

using boost::interprocess::managed_shared_memory;
using SegmentManager = managed_shared_memory::segment_manager;

template <typename T, typename SegmentManager = SegmentManager>
using Allocator = boost::interprocess::allocator<T, SegmentManager>;

class ShmManaged : private boost::noncopyable
{
public:
	bool Initialize(const std::string &key, size_t size)
	{
		// Create a managed shared memory
		shm_key_ = key;
		// shm_ = std::make_unique<managed_shared_memory>(boost::interprocess::open_or_create, shm_key_.c_str(), size);
		shm_.reset(new managed_shared_memory(boost::interprocess::open_or_create, shm_key_.c_str(), size));
		assert(shm_ != nullptr);

		// Create a allocator shared memory
		// shm_allocator_ = std::make_unique<Allocator<void>>(shm_->get_segment_manager());
		shm_allocator_.reset(new Allocator<void>(shm_->get_segment_manager()));
		assert(shm_allocator_ != nullptr);

		return shm_->get_size() == size;
	}

	managed_shared_memory &GetShmManaged()
	{
		return *shm_;
	}

	Allocator<void> &GetShmAllocator()
	{
		return *shm_allocator_;
	}

	void Grow(unsigned int size)
	{
		try
		{
			//Now that the segment is not mapped grow it adding extra ${size} bytes
			shm_->grow(shm_key_.c_str(), size);
		}
		catch (...)
		{
			std::cout << "grow error!" << std::endl;
		}
	}

	void ShrinkToFit()
	{
		try
		{
			//Now minimize the size of the segment
			shm_->shrink_to_fit(shm_key_.c_str());
		}
		catch (...)
		{
			std::cout << "shrink_to_fit error!" << std::endl;
		}
	}

private:
	std::string shm_key_;
    std::unique_ptr<managed_shared_memory> shm_;
	std::unique_ptr<Allocator<void>> shm_allocator_;
};
using ShmManagedSgl = boost::detail::thread::singleton<ShmManaged>;


//String
using String = boost::interprocess::basic_string<char
	, std::char_traits<char>
	, Allocator<char> >;

class ShmString : public String
{
public:
	ShmString() : String(ShmManagedSgl::instance().GetShmAllocator())
	{
	}

	ShmString(const ShmString&) = default; //默认调用基类的拷贝构造
	ShmString(ShmString&&) = default;

	String& operator=(const ShmString& other)
	{
		return String::operator=(other);
	}

	String& operator=(const std::string& other)
	{
		return String::operator=(other.c_str());
	}

	String& operator=(const char* str)
	{
		return String::operator=(str);
	}
};

// vector
template <typename T>
using Vector = boost::interprocess::vector< T, Allocator<T> >;

// ShmVector
template <typename T>
class ShmVector : public Vector<T>
{
public:
	ShmVector() : Vector<T>(ShmManagedSgl::instance().GetShmAllocator())
	{
	}

	ShmVector(const ShmVector&) = default; //默认调用基类的拷贝构造
	ShmVector(ShmVector&&) = default;

	ShmVector& operator=(const ShmVector& other)
	{
		Vector<T>::operator=(other);
		return *this;
	}

	ShmVector& operator=(ShmVector &&other)
	{
		Vector<T>::operator=(std::move(other));
		return *this;
	}
};

// list
template <typename T>
using List = boost::interprocess::list< T, Allocator<T> >;

// ShmList
template <typename T>
class ShmList : public List<T>
{
public:
	ShmList() : List<T>(ShmManagedSgl::instance().GetShmAllocator())
	{
	}

	ShmList(const ShmList&) = default; //默认调用基类的拷贝构造
	ShmList(ShmList&&) = default;

	ShmList& operator=(const ShmList& other)
	{
		List<T>::operator=(other);
		return *this;
	}

	ShmList& operator=(ShmList &&other)
	{
		List<T>::operator=(std::move(other));
		return *this;
	}
};

// pair
template <typename First, typename Second>
using Pair = std::pair<const First, Second>;

// ShmPair
template <typename First, typename Second>
class ShmPair : Pair<First, Second>
{
public:
	ShmPair() : Pair<First, Second>(ShmManagedSgl::instance().GetShmAllocator())
	{
	}

	ShmPair(const ShmPair&) = default; //默认调用基类的拷贝构造
	ShmPair(ShmPair&&) = default;

	ShmPair& operator=(const ShmPair& other)
	{
		Pair<First, Second>::operator=(other);
		return *this;
	}

	ShmPair &operator=(ShmPair &&other)
	{
		Pair<First, Second>::operator=(std::move(other));
		return *this;
	}
};

// map
template <typename First, typename Second, typename PairAllocator = Allocator < Pair<First, Second> > >
using Map = boost::interprocess::map < First, Second
	, std::less<First>
	, PairAllocator >;

// ShmMap
template <typename First, typename Second, typename PairAllocator = Allocator < Pair<First, Second> > >
class ShmMap : public Map<First, Second>
{
public:
	ShmMap() : Map<First, Second>(ShmManagedSgl::instance().GetShmAllocator())
	{
	}

	ShmMap(const ShmMap&) = default; //默认调用基类的拷贝构造
	ShmMap(ShmMap&&) = default;

	ShmMap& operator=(const ShmMap& other)
	{
		Map<First, Second>::operator=(other);
		return *this;
	}

	ShmMap& operator=(ShmMap &&other)
	{
		Map<First, Second>::operator=(std::move(other));
		return *this;
	}
};

} //namespace shm
} //namespace ipc 

#endif //ICP_SHM_CONTAINER_H