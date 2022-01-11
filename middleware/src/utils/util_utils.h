// util utility func interface

#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "util_defines.h"
#include "util_system.h"

namespace ipc {
namespace util {
namespace common {

// default 0=false
bool int_to_bool(const int32_t i, bool zero_as_false=true);

// catch invalid_argument, out_of_range exception
int32_t safe_stoi(const std::string& str);
int64_t safe_stoll(const std::string& str);
double safe_stod(const std::string& str);

// c++ basename
std::string basename(const std::string& filename);

// glob files
bool glob_files(const std::string& pattern, std::vector<std::string>& files);

} // namespace common
} // namespace util
} // namespace ipc
