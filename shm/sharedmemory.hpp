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
#include "shmcontainer.h"

namespace ipc {
namespace shm {

using namespace boost::interprocess;

template <typename T>
class SharedMemory
{
public:
    SharedMemory(managed_shared_memory &managed_shm) :
		managed_shm_{managed_shm}
	{
	}

    T *Construct(uint32_t key)
    {
        return Construct(std::to_string(key));
    }

    T *Construct(const std::string &key)
	{
		try
		{
			auto lamb = [this](const std::string &key) {
				managed_shm_.find_or_construct<T>(key.c_str())();
			};

			auto lamb_construct = std::bind(lamb, key);
			managed_shm_.atomic_func(lamb_construct);

			auto ptr = managed_shm_.find<T>(key.c_str());
			if (ptr.first != nullptr)
				return ptr.first;
		}
		catch (const bad_alloc &e)
		{
			std::cout << e.what() << std::endl;
			try { managed_shm_.grow(name_, shm_size_/*managed_shm_.get_user_size() * 2*/); }
			catch (...) { std::cout << "Fatal error!" << std::endl; }
		}
		catch (const interprocess_exception &e)
		{
			std::cout << e.what() << std::endl;
		}
		return nullptr;
	}

	template<typename Tp, typename ...Args>
    Tp *Construct(const std::string &key, Args &&...args)
	{
		try
		{
			auto lamb = [&](const std::string &key) {
				managed_shm_.find_or_construct<Tp>(key.c_str())(std::forward<Args>(args)...);
			};

			auto lamb_construct = std::bind(lamb, key);
			managed_shm_.atomic_func(lamb_construct, key);

			auto ptr = managed_shm_.find<Tp>(key.c_str());
			if (ptr.first != nullptr)
				return ptr.first;
		}
		catch (const bad_alloc &e)
		{
			std::cout << e.what() << std::endl;
			try { managed_shm_.grow(name_, shm_size_/*managed_shm_.get_user_size() * 2*/); }
			catch (...) { std::cout << "Fatal error!" << std::endl; }
		}
		catch (const interprocess_exception &e)
		{
			std::cout << e.what() << std::endl;
		}
		return nullptr;
	}

	std::pair<T*, unsigned int> GetPair(const std::string & key)
	{
		try
		{
			auto pair = managed_shm_.find<T>(key.c_str());
			if (pair.first != nullptr)
				return pair;
		}
		catch (const interprocess_exception &e)
		{
			std::cout << e.what() << std::endl;
		}
		return std::make_pair(nullptr, 0);
	}

    T *Open(uint32_t key)
    {
        return Open(std::to_string(key));
    }

    T *Open(const std::string & key)
	{
		auto pair = GetPair(key);
		if (pair.first != nullptr)
			return pair.first;

		return nullptr;
	}

    T const *OnlyReadOpen(uint32_t key)
    {
        return Open(std::to_string(key));
    }

    T const *OnlyReadOpen(const std::string & key) const
    {
        return Open(key);
    }

    void Destroy(const std::string & key)
    {
        try
        {
            managed_shm_.destroy<T>(key.c_str());
        }
        catch (interprocess_exception& e)
        {
            std::cout << e.what() << std::endl;
        }
    }

private:
	const char *name_;
	size_t shm_size_;
    managed_shared_memory &managed_shm_;
};

} // namespace shm
} // namespace ipc

#endif // ICP_SHARED_MEMORY_H
