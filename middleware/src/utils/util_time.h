// util system time interface

#pragma once
#include <cstdint>
#include <chrono>
#include <thread>

namespace ipc {
namespace util {
namespace common {

// sleep in milliseconds
void sleep_ms(const int64_t ms);

// sleep in microseconds（µs）
void sleep_us(const int64_t us);

// system clock now in milliseconds
int64_t now_ms();

// system clock now in microseconds（µs）
int64_t now_us();

// system clock now in yyyy-mm-dd
std::string now_date();

// system clock now in hh:mm:ss
std::string now_time();

// system clock now in yyyy-mm-dd.hh:mm:ss
std::string now_datetime();

} // namespace common
} // namespace util
} // namespace ipc
