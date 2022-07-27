#pragma once
#include <cstdio>
#include <cstdarg>
#include <string>

namespace ipc {
namespace util {

// trim string
std::string string_trim(const std::string& str);

// fill char* array by std::string
size_t string_to_char(const std::string& str, char* buff, const size_t buff_size);

// fill std::string by char*/void*
void char_to_string(const char* buff, const size_t buff_size, std::string& str);

// sprintf() like for std::string
std::string string_sprintf(const char* format, ...);

// sprintf() like for std::string
std::string string_vsnprintf(const std::string& format, va_list args);

// MISRA C++ 2008: 27-0-1
// stdio append for thread-safe
// file = "stderr", "stdout", file_path
void string_append(const std::string& file, const char* format, ...);

// load string to content from file
bool string_load(const std::string& file, std::string& content);

} // namespace util
} // namespace ipc
