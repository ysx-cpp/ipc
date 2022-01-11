#include <cstring>
#include <cassert>
#include <vector>
#include "util_string.h"

namespace ipc {
namespace util {
namespace common {

std::string string_trim(const std::string& str) {
    std::string s(str);
    s.erase(0, s.find_first_not_of(" \t\n\r\v\f"));
    s.erase(s.find_last_not_of(" \t\n\r\v\f") + 1);
    return s;
}

size_t string_to_char(const std::string& str, char* buff, const size_t buff_size) {
    memset(buff, 0, str.length());
    return str.copy(buff, buff_size);
}

void char_to_string(const char* buff, const size_t buff_size, std::string& str) {
    str.assign(buff, buff_size);
    str.resize(buff_size);
}

std::string string_sprintf(const char* format, ...) {
    va_list args;
    va_start(args, format);
    std::string buff = string_vsnprintf(format, args);
    va_end(args);
    return buff;
}

std::string string_vsnprintf(const std::string& format, va_list args) {
    // try first
    int32_t try_size = 512;
    char try_buff[try_size];
    memset(try_buff, 0, try_size);

    std::string res;
    int32_t size_needed = std::vsnprintf(try_buff, try_size, format.c_str(), args);
    if (size_needed < try_size) {
        res = try_buff;
    } else {
        // try again
        char buff_needed[size_needed+1];
        memset(buff_needed, 0, size_needed+1);
        int32_t size = std::vsnprintf(buff_needed, size_needed+1, format.c_str(), args);
        if (size >= 0) {
            res = std::string(buff_needed);
        } else {
            res = std::string("");
        }
    }

    // MISRA C++ 2008: 6-6-5
    return res;
}

void _string_append(const std::string& file, const char* format, va_list ap) {
    assert(file != "");

    FILE* fp = nullptr;
    if (file == "stdout") {
        fp = stdout;
    } else if (file == "stderr") {
        fp = stderr;
    } else {
        fp = fopen(file.c_str(), "a");
    }

    if (fp != nullptr) {
        std::vfprintf(fp, format, ap);

        if ((file != "stdout") && (file != "stderr")){
            fclose(fp);
            fp = nullptr;
        }
    }
}

void string_append(const std::string& file, const char* format, ...) {
    assert(file != "");

    va_list ap;
    va_start(ap, format);
    _string_append(file, format, ap);
    va_end(ap);
}

bool string_load(const std::string& file, std::string& content) {
    assert(file != "");

    FILE* fp = fopen(file.c_str(), "rb");
    if (fp == nullptr) {
        return false;
    }

    // check size
    fseek(fp, 0, SEEK_END);
    size_t bufsize = ftell(fp);
    rewind(fp);

    // malloc buffer
    char buf[bufsize + 1];
    memset(buf, 0, bufsize + 1);
    size_t res = fread(buf, 1, bufsize, fp);
    fclose(fp);

    if (res <= 0) {
        return false;
    }

    // to string
    char_to_string(buf, bufsize, content);
    return true;
}

} // namespace common
} // namespace util
} // namespace ipc
