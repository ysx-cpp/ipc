/*
 * @file shmcontainer.h
 * @author Songxi Yang
 * @mail ysx-cpp@gmail.com
 * @github https://github.com/ysx-cpp
 * @date Oct 08 2021
 */
#ifndef ICP_SHM_CONTAINER_H
#define ICP_SHM_CONTAINER_H
#include <boost/interprocess/containers/map.hpp>
#include <boost/interprocess/containers/vector.hpp>
#include <boost/interprocess/containers/list.hpp>
#include <boost/interprocess/containers/string.hpp>
#include "shmmanaged.h"

namespace ipc {
namespace shm {

//String
using String = boost::interprocess::basic_string<char
	, std::char_traits<char>
	, ShmAllocator<char> >;

class ShmString : public String
{
public:
	ShmString() : String(ShmManagedSingle::instance().GetShmAllocator())
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
using Vector = boost::interprocess::vector< T, ShmAllocator<T> >;

// ShmVector
template <typename T>
class ShmVector : public Vector<T>
{
public:
	ShmVector() : Vector<T>(ShmManagedSingle::instance().GetShmAllocator())
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
using List = boost::interprocess::list< T, ShmAllocator<T> >;

// ShmList
template <typename T>
class ShmList : public List<T>
{
public:
	ShmList() : List<T>(ShmManagedSingle::instance().GetShmAllocator())
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
	ShmPair() : Pair<First, Second>(ShmManagedSingle::instance().GetShmAllocator())
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
template <typename First, typename Second, typename PairAllocator = ShmAllocator < Pair<First, Second> > >
using Map = boost::interprocess::map < First, Second
	, std::less<First>
	, PairAllocator >;

// ShmMap
template <typename First, typename Second, typename PairAllocator = ShmAllocator < Pair<First, Second> > >
class ShmMap : public Map<First, Second>
{
public:
	ShmMap() : Map<First, Second>(ShmManagedSingle::instance().GetShmAllocator())
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