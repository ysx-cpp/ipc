// util thread-pool interface
// support: interruptible, priority, affinity, naming
// limit: parallel threads, cpu usage, memory usage

#pragma once
#include <vector>
#include <queue>
#include <memory>
#include <atomic>
#include <thread>
#include <future>
#include <functional>
#include <condition_variable>
#include "util_defines.h"
#include "util_system.h"

namespace ipc {
namespace util {
namespace common {

class ThreadPool {

// task name -> task function
using ThreadPoolTask = std::pair<std::string, std::function<void()>>;

public:
    ThreadPool(const size_t workers_limit, const size_t queue_limit, const int32_t pop_strategy) {
        assert(workers_limit > 0);
        assert(queue_limit > 0);
        assert(pop_strategy > 0);

        m_exit_signal = false;
        m_queue_limit = queue_limit;
        m_pop_strategy = pop_strategy;

        auto worker = std::bind(&ThreadPool::worker, this);
        for (size_t i = 0; i < workers_limit; ++i) {
            m_threads.emplace_back(worker);
        }
    }

    ~ThreadPool() {
        m_exit_signal = true;
        m_condition.notify_all();

        for (auto& thread : m_threads) {
            if (thread.joinable()) {
                thread.join();
            }
        }
    }

    template <typename Func, typename... Args>
    auto push(const std::string& task_name, Func&& func, Args&&... args)
        -> std::future<typename std::result_of<Func(Args...)>::type> {

        using return_type = typename std::result_of<Func(Args...)>::type;
        auto task = std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<Func>(func), std::forward<Args>(args)...));

        std::future<return_type> res = task->get_future();

        if (m_exit_signal) {
            return std::future<return_type>();
        }

        //         [front ... back]
        // push -> [begin ...  end] -> pop
        //         [ 0    ...   n ]
        std::unique_lock<std::mutex> lock(m_queue_mutex);
        m_queue.emplace_front(std::make_pair(task_name, [task]() {(*task)();}));
        if (m_queue.size() > m_queue_limit) {
            m_queue.pop_back();
        }
        m_condition.notify_one();
        return res;
    }

private:
    void worker() {
        ThreadPoolTask task;
        while (m_exit_signal == false) {

            // lock and waiting for new task
            std::unique_lock<std::mutex> lock(m_queue_mutex);
            while (m_queue.empty() && (m_exit_signal == false)) {
                m_condition.wait(lock);
            }

            // exit signal
            if (m_exit_signal) {
                lock.unlock();
                return;
            }

            // pop task
            if (m_pop_strategy == ipc_THREAD_POOL_DROP) {
                task = std::move(m_queue.front());
                m_queue.pop_front();
            } else { // ipc_THREAD_POOL_FIFO
                task = std::move(m_queue.back());
                m_queue.pop_back();
            }

            // unlock
            lock.unlock();

            if (task.first != "") {
                ipc::util::common::set_thread_name(task.first);
            }

            // run
            task.second();
        }
    }

private:
    int32_t m_pop_strategy;
    size_t m_queue_limit;
    std::mutex m_queue_mutex;
    std::condition_variable m_condition;
    std::deque<ThreadPoolTask> m_queue;
    std::vector<std::thread> m_threads;
    std::atomic<bool> m_exit_signal;
};

} // namespace common
} // namespace util
} // namespace ipc
