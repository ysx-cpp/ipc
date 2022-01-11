// util system time implement

#include <ctime>
#include "util_string.h"
#include "util_time.h"

namespace ipc {
namespace util {
namespace common {

// sleep in milliseconds
void sleep_ms(const int64_t ms) {
#ifdef __QNXNTO__
    std::this_thread::sleep_for(std::chrono::milliseconds(ms - 1));
#else
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
#endif
}

// sleep in microseconds（µs）
void sleep_us(const int64_t us) {
    std::this_thread::sleep_for(std::chrono::microseconds(us));
}

// system clock now in milliseconds
int64_t now_ms() {
    auto now = std::chrono::high_resolution_clock::now();
    auto time_point = std::chrono::time_point_cast<std::chrono::milliseconds>(now);
    auto epoch = time_point.time_since_epoch();
    return epoch.count();
}

// system clock now in microseconds（µs）
int64_t now_us() {
    auto now = std::chrono::high_resolution_clock::now();
    auto time_point = std::chrono::time_point_cast<std::chrono::microseconds>(now);
    auto epoch = time_point.time_since_epoch();
    return epoch.count();
}

// system clock now in yyyy-mm-dd
std::string now_date() {
    auto now = std::chrono::system_clock::now();
    auto _time = std::chrono::system_clock::to_time_t(now);

    // MISRA C++ 2008: 18-0-4
    struct tm _tm;
    std::string date_str;

    if (localtime_r(&_time, &_tm) != nullptr) {
        date_str = string_sprintf("%04d-%02d-%02d", _tm.tm_year + 1900, _tm.tm_mon + 1, _tm.tm_mday);
    } else {
        date_str = "1970-01-01";
    }

    // MISRA C++ 2008: 6-6-5
    return date_str;
}

// system clock now in hh:mm:ss
std::string now_time() {
    auto now = std::chrono::system_clock::now();
    auto _time = std::chrono::system_clock::to_time_t(now);

    // MISRA C++ 2008: 18-0-4
    struct tm _tm;
    std::string time_str;

    if (localtime_r(&_time, &_tm) != nullptr) {
        time_str = string_sprintf("%02d:%02d:%02d", _tm.tm_hour, _tm.tm_min, _tm.tm_sec);
    } else {
        time_str = "00:00:00";
    }

    // MISRA C++ 2008: 6-6-5
    return time_str;
}

// system clock now in yyyy-mm-dd.hh:mm:ss
std::string now_datetime() {
    auto now = std::chrono::system_clock::now();
    auto _time = std::chrono::system_clock::to_time_t(now);

    // MISRA C++ 2008: 18-0-4
    struct tm _tm;
    std::string datetime_str;

    if (localtime_r(&_time, &_tm) != nullptr) {
        datetime_str = string_sprintf("%04d-%02d-%02d.%02d:%02d:%02d",
                                      _tm.tm_year + 1900, _tm.tm_mon + 1, _tm.tm_mday,
                                      _tm.tm_hour, _tm.tm_min, _tm.tm_sec);
    } else {
        datetime_str = "1970-01-01.00:00:00";
    }

    // MISRA C++ 2008: 6-6-5
    return datetime_str;
}

} // namespace common
} // namespace util
} // namespace ipc
