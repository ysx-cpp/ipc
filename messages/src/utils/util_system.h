#pragma once
#include <cstdint>
#include <string>

namespace ipc {
namespace util {

// get current process name
std::string get_process_name();

// get current thread id
int64_t get_thread_id();

// set current thread name, max length <= 15
bool set_thread_name(const std::string& name);

// get env, check null
std::string safe_getenv(const std::string& env);

} // namespace util
} // namespace ipc
