/*
 * @file sharedmemory.hpp
 * @author Songxi Yang
 * @mail ysx-cpp@gmail.com
 * @github https://github.com/ysx-cpp
 * @date Oct 08 2021
 */
#ifndef ICP_SHARED_MEMORY_H
#define ICP_SHARED_MEMORY_H

#include <iostream>
#include <string>
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/managed_shared_memory.hpp>
#include "shmmanaged.h"

namespace ipc {
namespace shm {

using boost::interprocess::managed_shared_memory;
using boost::interprocess::interprocess_exception;
using boost::interprocess::bad_alloc;

#define __SHM_TRY__ \
try                 \
{
#define __SHM_CATCH__                                                           \
}                                                                               \
catch (const bad_alloc &e)                                                      \
{                                                                               \
ShmManagedSingle::instance().Grow();                                            \
std::cerr << e.what() << std::endl;                                             \
}                                                                               \
catch (const interprocess_exception &e) { std::cerr << e.what() << std::endl; } \
catch (...) { std::cerr << "Unknown error" << std::endl; }

template <typename T>
class SharedMemory
{
public:
    SharedMemory() :
		managed_shm_{ShmManagedSingle::instance().GetShmManaged()}
	{
	}
    
	T *Construct(int64_t key)
    {
        return Construct(std::to_string(key));
    }

    T *Construct(const std::string &key)
	{
		__SHM_TRY__
			auto lamb = [this](const std::string &key) {
				managed_shm_.find_or_construct<T>(key.c_str())();
			};

			auto lamb_construct = std::bind(lamb, key);
			managed_shm_.atomic_func(lamb_construct);

			auto ptr = managed_shm_.find<T>(key.c_str());
			if (ptr.first != nullptr)
				return ptr.first;
		__SHM_CATCH__

		return nullptr;
	}

	template<typename Tp, typename ...Args>
    Tp *Construct(const std::string &key, Args &&...args)
	{
		__SHM_TRY__
			auto lamb = [&](const std::string &key) {
				managed_shm_.find_or_construct<Tp>(key.c_str())(std::forward<Args>(args)...);
			};

			auto lamb_construct = std::bind(lamb, key);
			managed_shm_.atomic_func(lamb_construct, key);

			auto ptr = managed_shm_.find<Tp>(key.c_str());
			if (ptr.first != nullptr)
				return ptr.first;
		__SHM_CATCH__

		return nullptr;
	}

	std::pair<T*, unsigned int> GetShmObj(const std::string & key)
	{
		__SHM_TRY__
			auto pair = managed_shm_.find<T>(key.c_str());
			if (pair.first != nullptr)
				return pair;
		__SHM_CATCH__

		return std::make_pair(nullptr, 0);
	}

    T *Open(int64_t key)
    {
        return Open(std::to_string(key));
    }

    T *Open(const std::string & key)
	{
		auto pair = GetShmObj(key);
		if (pair.first != nullptr)
			return pair.first;

		return nullptr;
	}

    T const *OnlyReadOpen(int64_t key)
    {
        return Open(std::to_string(key));
    }

    T const *OnlyReadOpen(const std::string & key) const
    {
        return Open(key);
    }

	void Destroy(const std::string & key)
    {
        __SHM_TRY__
            managed_shm_.destroy<T>(key.c_str());
        __SHM_CATCH__
    }

private:
    managed_shared_memory &managed_shm_;
};

} // namespace shm
} // namespace ipc

#endif // ICP_SHARED_MEMORY_H
