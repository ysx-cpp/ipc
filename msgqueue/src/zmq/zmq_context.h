// zeromq context singleton interface

#pragma once
#include <memory>
//#include "utils/util_common.h"
#include "utils/util_defines.h"
#include "zmq.hpp"

namespace ipc {
namespace messages {

class ContextManager {

public:
    static zmq::context_t& instance() {
        static zmq::context_t s_zmq_ctx = zmq::context_t(ipc_ZMQ_IO_THREADS);
        return s_zmq_ctx;
    }

    // disable copying
    ContextManager(ContextManager const&) = delete;
    void operator=(ContextManager const&) = delete;

private:
    ContextManager() {}
    ~ContextManager() {}
};

} // namespace messages
} // namespace ipc
