/*
 * @file shmlock.h
 * @author Songxi Yang
 * @mail ysx-cpp@gmail.com
 * @github https://github.com/ysx-cpp
 * @date Oct 08 2021
 */
#ifndef ICP_SHM_LOCK_H
#define ICP_SHM_LOCK_H
#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <boost/interprocess/sync/interprocess_condition.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>

namespace ipc {
namespace shm {

using ShmMutex = boost::interprocess::interprocess_mutex;
using ShmCondition = boost::interprocess::interprocess_condition;
using ShmScopedLock = boost::interprocess::scoped_lock<ShmMutex>;

} //namespace shm
} //namespace ipc

#endif //ICP_SHM_LOCK_H