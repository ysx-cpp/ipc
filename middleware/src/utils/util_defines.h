#pragma once

////////////////////////////////////////////////////////////////////////////////
// config path and files

#define ENV_CONFIG_PATH             "ipc_CONFIG_PATH" // set by ipc_setup.sh
#define ipc_CONFIG_MESSAGES        "messages.config"
#define ipc_CONFIG_DICT            "dict.config"
#define ipc_CONFIG_LOGGING         "logging.config"
#define ipc_CONFIG_SCHEDULER       "scheduler.config"

////////////////////////////////////////////////////////////////////////////////
// thread-pool pop strategy
#define ipc_THREAD_POOL_FIFO       1
#define ipc_THREAD_POOL_DROP       2

////////////////////////////////////////////////////////////////////////////////
// messages/zmq settings

#define ipc_ZMQ_IO_THREADS         1

////////////////////////////////////////////////////////////////////////////////
// dict settings

#define ENV_DICT_PATH				"ipc_DICT_PATH" // set by ipc_setup.sh
#define ipc_BDB_BUFFER_SIZE        1024

////////////////////////////////////////////////////////////////////////////////
// logging settings

// mode options
#define ipc_LOGGING_MODE_LOCAL     1
#define ipc_LOGGING_MODE_ASYNC     2
#define ipc_LOGGING_MODE_REMOTE    3
// level options
#define ipc_LOGGING_LEVEL_INFO     1
#define ipc_LOGGING_LEVEL_WARN     2
#define ipc_LOGGING_LEVEL_ERROR    3
#define ipc_LOGGING_LEVEL_NONE     4
// stderr color code
#define ipc_LOGGING_GREEN          "\033[0;32m"
#define ipc_LOGGING_YELLOW         "\033[0;33m"
#define ipc_LOGGING_RED            "\033[0;31m"
#define ipc_LOGGING_WHITE          "\033[0;37m"
// logging env settings
#define ENV_LOGGING_PATH            "ipc_LOGGING_PATH" // set by ipc_setup.sh
#define ENV_LOGGING_MODE            "ipc_LOGGING_MODE"
#define ENV_LOGGING_LEVEL           "ipc_LOGGING_LEVEL"
#define ENV_LOGGING_STDERR          "ipc_LOGGING_STDERR"
#define ENV_LOGGING_COLOR           "ipc_LOGGING_COLOR"

////////////////////////////////////////////////////////////////////////////////
// scheduler settings

// scheduler debug topic
#define ipc_TASK_INTERNAL_EVENT    "util-scheduler-internal-event"
#define ipc_TASK_EXTERN_EVENT      "util-scheduler-extern-event"
