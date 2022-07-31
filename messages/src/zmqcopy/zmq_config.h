// zeromq config

#pragma once

// server addr
#define ipc_SERVER_REP_ADDR     "*"
#define ipc_SERVER_PUB_ADDR     "*"
#define ipc_SERVER_REQ_ADDR     "127.0.0.1"
#define ipc_SERVER_SUB_ADDR     "127.0.0.1"

// topic port
#define ipc_TOPIC_START_PORT    7714

// buffer and waiting
#define ipc_ZMQ_SEND_QUEUE      10000
#define ipc_ZMQ_RECV_QUEUE      10000
#define ipc_ZMQ_CLOSE_WAIT      0
#define ipc_ZMQ_POOL_TIMEOUT    10

// thread-pool
#define ipc_THREAD_WORKERS      10
#define ipc_THREAD_QUEUE_SIZE   1000
