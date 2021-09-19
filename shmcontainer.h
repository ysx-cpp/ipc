/*
 * @file shmcontainer.h
 * @author Songxi Yang
 * @mail ysx-cpp@gmail.com
 * @github https://github.com/ysx-cpp
 * @date Oct 08 2021
 */
#ifndef ICP_SHM_CONTAINER_H
#define ICP_SHM_CONTAINER_H
#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/interprocess/containers/map.hpp>
#include <boost/interprocess/containers/vector.hpp>
#include <boost/interprocess/containers/list.hpp>
#include <boost/interprocess/containers/string.hpp>

namespace ipc {

using boost::interprocess::managed_shared_memory;
using SegmentManager = managed_shared_memory::segment_manager;

template <typename T, typename SegmentManager = SegmentManager>
using Allocator = boost::interprocess::allocator<T, SegmentManager>;

static managed_shared_memory g_shm_manager(boost::interprocess::open_or_create, "DefaultName", 0x80000000);
static Allocator<void> g_shm_allocator(g_shm_manager.get_segment_manager());


//string
using String = boost::interprocess::basic_string<char
	, std::char_traits<char>
	, Allocator<char> >;

class CString : public String
{
public:
	CString() : String(g_shm_allocator)
	{
	}

	CString(const CString&) = default; //默认调用基类的拷贝构造
	CString(CString&&) = default;

	String& operator=(const CString& other)
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

// CVector
template <typename T>
class CVector : public Vector<T>
{
public:
	CVector() : Vector<T>(g_shm_allocator)
	{
	}

	CVector(const CVector&) = default; //默认调用基类的拷贝构造
	CVector(CVector&&) = default;

	CVector& operator=(const CVector& other)
	{
		return Vector::operator=(other);
	}
};

// list
template <typename T>
using List = boost::interprocess::list< T, Allocator<T> >;

// CList
template <typename T>
class CList : public List<T>
{
public:
	CList() : List<T>(g_shm_allocator)
	{
	}

	CList(const CList&) = default; //默认调用基类的拷贝构造
	CList(CList&&) = default;

	CList& operator=(const CList& other)
	{
		return List::operator=(other.c_str());
	}
};

// pair
template <typename First, typename Second>
using Pair = std::pair<const First, Second>;

// CPair
template <typename First, typename Second>
class CPair : Pair<First, Second>
{
public:
	CPair() : Pair<First, Second>(g_shm_allocator)
	{
	}

	CPair(const CPair&) = default; //默认调用基类的拷贝构造
	CPair(CPair&&) = default;

	CPair& operator=(const CPair& other)
	{
		return Pair::operator=(other.c_str());
	}
};

// map
template <typename First, typename Second, typename PairAllocator = Allocator < Pair<First, Second> > >
using Map = boost::interprocess::map < First, Second
	, std::less<First>
	, PairAllocator >;

// CMap
template <typename First, typename Second, typename PairAllocator = Allocator < Pair<First, Second> > >
class CMap : public Map<First, Second>
{
public:
	CMap() : Map<First, Second>(g_shm_allocator)
	{
	}

	CMap(const CMap&) = default; //默认调用基类的拷贝构造
	CMap(CMap&&) = default;

	CMap& operator=(const CMap& other)
	{
		return Map::operator=(other.c_str());
	}
};

} //namespace ipc 

#endif //ICP_SHM_CONTAINER_H