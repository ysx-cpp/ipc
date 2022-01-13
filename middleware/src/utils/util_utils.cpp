// util utility func implement

#include <glob.h>
#include <cstdarg>
#include <cstring>
#include <vector>
#include <stdexcept>
#include "util_utils.h"

namespace ipc {
namespace util {

bool int_to_bool(const int32_t i, bool zero_as_false) {
    bool res = false;
    if (zero_as_false && (i != 0)) {
        res = true;
    } else if ((!zero_as_false) && (i == 0)) {
        res = true;
    }
    // MISRA C++ 2008: 6-6-5
    return res;
}

int32_t safe_stoi(const std::string& str) {
    int32_t res = 0;
    try {
        res = std::stoi(str);
    } catch (std::out_of_range& e) {
        res = 0;
    } catch (std::invalid_argument& e) {
        res = 0;
    }
    // MISRA C++ 2008: 6-6-5
    return res;
}

int64_t safe_stoll(const std::string& str) {
    int64_t res = 0;
    try {
        res = std::stoll(str);
    } catch (std::out_of_range& e) {
        res = 0;
    } catch (std::invalid_argument& e) {
        res = 0;
    }
    // MISRA C++ 2008: 6-6-5
    return res;
}

double safe_stod(const std::string& str) {
    double res = 0.0;
    try {
        res = std::stod(str);
    } catch (std::out_of_range& e) {
        res = 0.0;
    } catch (std::invalid_argument& e) {
        res = 0.0;
    }
    // MISRA C++ 2008: 6-6-5
    return res;
}

std::string basename(const std::string& filename) {
    // MISRA C++ 2008: 6-4-2
    size_t p1 = filename.rfind("/");
    size_t p2 = filename.rfind("\\");
    std::string name;
    if (p1 != filename.npos) {
        name = filename.substr(p1 + 1);
    } else if (p2 != filename.npos) {
        name = filename.substr(p2 + 1);
    } else {
        name = filename;
    }
    // MISRA C++ 2008: 6-6-5
    return name;
}

bool glob_files(const std::string& pattern, std::vector<std::string>& files) {
    glob_t glob_result;
    memset(&glob_result, 0, sizeof(glob_result));

    int res = glob(pattern.c_str(), GLOB_TILDE, nullptr, &glob_result);
    if (res != 0) {
        globfree(&glob_result);
        return false;
    }

    for (size_t i = 0; i < glob_result.gl_pathc; ++i) {
        files.push_back(std::string(glob_result.gl_pathv[i]));
    }

    globfree(&glob_result);
    return true;
}

} // namespace util
} // namespace ipc
