#pragma once
#include <string>
#include "util_defines.h"

#define DEBUG_INFO(format, ...) \
    do { \
        ipc::util::ipc_debug(ipc_LOGGING_LEVEL_INFO, \
            __FILE__, __LINE__, __func__, (format), ##__VA_ARGS__); \
    } while (0)

#define DEBUG_WARN(format, ...) \
    do { \
        ipc::util::ipc_debug(ipc_LOGGING_LEVEL_WARN, \
            __FILE__, __LINE__, __func__, (format), ##__VA_ARGS__); \
    } while (0)

#define DEBUG_ERROR(format, ...) \
    do { \
        ipc::util::ipc_debug(ipc_LOGGING_LEVEL_ERROR, \
            __FILE__, __LINE__, __func__, (format), ##__VA_ARGS__); \
    } while (0)

#define DEBUG_FUNC() \
    do { \
        ipc::util::ipc_debug(ipc_LOGGING_LEVEL_INFO, \
            __FILE__, __LINE__, __PRETTY_FUNCTION__, ""); \
    } while (0)

#define DEBUG_INFO_IF(condition, format, ...) \
    do { \
        if (condition) { \
            ipc::util::ipc_debug(ipc_LOGGING_LEVEL_INFO, \
                __FILE__, __LINE__, __func__, (format), ##__VA_ARGS__); \
        } \
    } while (0)

#define DEBUG_WARN_IF(condition, format, ...) \
    do { \
        if (condition) { \
            ipc::util::ipc_debug(ipc_LOGGING_LEVEL_WARN, \
                __FILE__, __LINE__, __func__, (format), ##__VA_ARGS__); \
        } \
    } while (0)

#define DEBUG_ERROR_IF(condition, format, ...) \
    do { \
        if (condition) { \
            ipc::util::ipc_debug(ipc_LOGGING_LEVEL_ERROR, \
                __FILE__, __LINE__, __func__, (format), ##__VA_ARGS__); \
        } \
    } while (0)

namespace ipc {
namespace util {

std::string level_to_color(const int32_t level);
std::string level_to_string(const int32_t level);

void ipc_debug(const int32_t level, const std::string& file, const int32_t line,
    const std::string& func, const char* format, ...);

} // namespace util
} // namespace ipc
