#ifdef __QNXNTO__
#include <nbutil.h>         // for getprogname()
#include <process.h>        // for gettid()
#include <backtrace.h>      // for bt_get_backtrace()
#include <sys/types.h>
#include <sys/neutrino.h>   // for ThreadCtl()
#include <sys/procmgr.h>    // for procmgr_ability()
#include <devctl.h>         // for syspage
#include <sys/neutrino.h>
#include <sys/syspage.h>
#else
#include <unistd.h>
#include <sys/syscall.h>    // for syscall()
#include <cerrno>           // for program_invocation_short_name
#endif

#include <pthread.h>
#include <cstdlib>
#include <cstring>
#include <cassert>
#include <thread>
#include "util_system.h"

namespace ipc {
namespace util {
namespace common {

std::string get_process_name() {
    std::string name;
#ifdef __QNXNTO__
    name = ::getprogname();
#else
    name = program_invocation_short_name;
#endif
    // MISRA C++ 2008: 6-6-5
    return name;
}

int64_t get_thread_id() {
    int64_t id = 0;
#ifdef __QNXNTO__
    id = ::gettid();
#else
    id = ::syscall(__NR_gettid);
#endif
    // MISRA C++ 2008: 6-6-5
    return id;
}

bool set_thread_name(const std::string& name) {
    assert(name != "");

    std::string resized_name(name);
    if (resized_name.length() > 15) {
        resized_name.resize(15); // limit of pthread_setname_np()
    }

    if (pthread_setname_np(pthread_self(), resized_name.c_str()) == 0) {
        return true;
    } else {
        return false;
    }
}

std::string safe_getenv(const std::string& env) {
    // MISRA C++ 2008: 18-0-3
    // no getenv_s()
    std::string val;
    char* buff = getenv(env.c_str());
    if (buff != nullptr) {
        val = buff;
    } else {
        val = "";
    }
    // MISRA C++ 2008: 6-6-5
    return val;
}

} // namespace common
} // namespace util
} // namespace ipc
