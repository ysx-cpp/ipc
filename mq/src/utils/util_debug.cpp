#include <cstdio>
#include "util_string.h"
#include "util_time.h"
#include "util_system.h"
#include "util_utils.h"
#include "util_debug.h"

namespace ipc {
namespace util {
namespace common {

std::string level_to_color(const int32_t level) {
    switch (level) {
        case ipc_LOGGING_LEVEL_INFO: return ipc_LOGGING_GREEN;
        case ipc_LOGGING_LEVEL_WARN: return ipc_LOGGING_YELLOW;
        case ipc_LOGGING_LEVEL_ERROR: return ipc_LOGGING_RED;
        default: return ipc_LOGGING_WHITE;
    }
}

std::string level_to_string(const int32_t level) {
    switch (level) {
        case ipc_LOGGING_LEVEL_INFO: return "INFO";
        case ipc_LOGGING_LEVEL_WARN: return "WARN";
        case ipc_LOGGING_LEVEL_ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}

void ipc_debug(const int32_t level, const std::string& file, const int32_t line,
    const std::string& func, const char* format, ...) {

    int32_t env_stderr_level = safe_stoi(safe_getenv(ENV_LOGGING_STDERR));
    if (level < env_stderr_level) {
        return;
    }

    va_list args;
    va_start(args, format);
    std::string text = string_vsnprintf(format, args);
    va_end(args);

    // line: level.datetime.thread_id.func: text
    std::string content = string_sprintf("%s[%s %s %d %s] %s%s", level_to_color(level).c_str(),
        level_to_string(level).c_str(), now_datetime().c_str(), get_thread_id(), func.c_str(),
        text.c_str(), ipc_LOGGING_WHITE);
    fprintf(stderr, "%s\n", content.c_str());
}

} // namespace common
} // namespace util
} // namespace ipc
