// util messages service client implement

#include <memory>
#include "zmq.hpp"
#include "zmq_context.h"
#include "zmq_envelope.h"
#include "messages_client.h"
#include "messages_protos.pb.h"

#include <google/protobuf/io/zero_copy_stream_impl.h>
//#include <gflags/gflags.h>

//DEFINE_string(home_dir, "", "eda home dir");

namespace ipc {
namespace util {
namespace messages {

static MessagesConfig s_config;

////////////////////////////////////////////////////////////////////////////////
MessagesClient::MessagesClient(const std::string& client_id, const std::string server_addr, const uint16_t server_port) {
    assert(client_id != "");

    DEBUG_INFO("create MessagesClient, client_id = %s, addr = %s, port = %u", client_id.c_str(), server_addr.c_str(), server_port);
    m_id = client_id;
    m_callback_mode = CallbackMode::sync;
    m_have_subscription = false;
    m_exit_signal = false;

    // load config
    bool load_config_success = false;
    
    if (s_config.ByteSizeLong() > 0) {
        load_config_success = true;
    } else {
        //if (true FLAGS_home_dir != "") 
        {
            //std::string ipc_config_file = FLAGS_home_dir + "/../util/config/ipc_config.prototxt";
            std::string ipc_config_file = "../config/ipc_config.prototxt";
            
            std::ifstream in(ipc_config_file);
            if (in) {
                google::protobuf::io::IstreamInputStream iis(&in);
                if (google::protobuf::TextFormat::Parse(&iis, &s_config)) {
                    load_config_success = true;
                } else {
                    DEBUG_ERROR("read_prototxt failed %s", ipc_config_file.c_str());
                }       
            } else {
                DEBUG_ERROR("failed to open ipc_config_file %s", ipc_config_file.c_str());
            }   
        }
    }

    if (load_config_success) {
        DEBUG_INFO("load_config success: %s", s_config.ShortDebugString().c_str());
        m_send_queue = s_config.zmq_send_queue();
        m_recv_queue = s_config.zmq_recv_queue();
        m_close_wait = s_config.zmq_close_wait();
        m_pool_timeout = s_config.zmq_pool_timeout();
        m_thread_workers = s_config.thread_workers();
        m_thread_queue_size = s_config.thread_queue_size();
    } else {
        DEBUG_ERROR("load_config failed");
        m_send_queue = 10000;
        m_recv_queue = 10000;
        m_close_wait = 0;
        m_pool_timeout = 10;
        m_thread_workers = 10;
        m_thread_queue_size = 1000;
    }
    
    // load config
    // m_send_queue = ipc_ZMQ_SEND_QUEUE;
    // m_recv_queue = ipc_ZMQ_RECV_QUEUE;
    // m_close_wait = ipc_ZMQ_CLOSE_WAIT;
    // m_pool_timeout = ipc_ZMQ_POOL_TIMEOUT;
    // m_thread_workers = ipc_THREAD_WORKERS;
    // m_thread_queue_size = ipc_THREAD_QUEUE_SIZE;

    // routing service client
    auto online_func = std::bind(&MessagesClient::publisher_online, this,
        std::placeholders::_1, std::placeholders::_2);
    auto offline_func = std::bind(&MessagesClient::publisher_offline, this,
        std::placeholders::_1, std::placeholders::_2);

    m_routing_client = std::make_shared<RoutingClient>(m_id, server_addr, server_port);
    m_routing_client->event_callback(online_func, offline_func);

    // start subscribe socket
    zmq::context_t& zmq_ctx = ContextManager::instance();
    m_sub_socket = std::make_shared<zmq::socket_t>(zmq_ctx, ZMQ_SUB);
    m_sub_socket->setsockopt(ZMQ_RCVHWM, m_recv_queue);
    m_sub_socket->setsockopt(ZMQ_LINGER, m_close_wait);

    // start thread-pool
    m_thread_pool = std::make_shared<ThreadPool>(
        m_thread_workers, m_thread_queue_size, ipc_THREAD_POOL_FIFO);
}

MessagesClient::~MessagesClient() {
    this->stop();
}

////////////////////////////////////////////////////////////////////////////////
template<>
bool MessagesClient::publish<std::string>(const std::string& topic,
    const std::string& message) {
    assert(topic != "");

    // lookup or advertise
    std::shared_ptr<zmq::socket_t> pub_socket;
    auto iter = m_pub_socket.find(topic);
    if (iter == m_pub_socket.end()) {
        std::string pub_addr;
        if (m_routing_client->lookup_pub_addr(topic, pub_addr) == false) {
            return false;
        }

        try {
            zmq::context_t& zmq_ctx = ContextManager::instance();
            pub_socket = std::make_shared<zmq::socket_t>(zmq_ctx, ZMQ_PUB);
            pub_socket->bind(pub_addr.c_str());
            pub_socket->setsockopt(ZMQ_SNDHWM, m_send_queue);
            pub_socket->setsockopt(ZMQ_LINGER, m_close_wait);
            m_pub_socket[topic] = pub_socket;
        } catch (zmq::error_t& e) {
            DEBUG_ERROR(e.what());
            return false;
        }

    } else {
        pub_socket = iter->second;
    }

    // send
    ++(m_pub_serial[topic]);
    MessageEnvelope msg(topic, m_id, message, m_pub_serial[topic]);
    std::lock_guard<std::mutex> lock(this->pub_mutex(topic));
    return msg.send(*pub_socket);
}

template<>
bool MessagesClient::subscribe<std::string>(
    const std::string& topic, std::function<void(const std::string&)> callback_func) {
    assert(topic != "");
    CallbackInterface callback_ptr = std::make_shared<MessageCallback1<std::string>>(callback_func);
    return register_subscription(topic, callback_ptr);
}

template<>
bool MessagesClient::subscribe<std::string, MessageHeader>(
    const std::string& topic, std::function<void(const std::string&, const MessageHeader&)> callback_func) {
    assert(topic != "");
    CallbackInterface callback_ptr = std::make_shared<MessageCallback2<std::string, MessageHeader>>(callback_func);
    return register_subscription(topic, callback_ptr);
}

bool MessagesClient::unsubscribe(const std::string& topic) {
    assert(topic != "");
    // sync with listening thread
    std::lock_guard<std::mutex> lock(m_sub_mutex);

    auto iter = m_subscribed.find(topic);
    if (iter == m_subscribed.end()) {
        // not subscribed yet
        return false;
    }

    // shoudown, then clean callback
    m_sub_socket->setsockopt(ZMQ_UNSUBSCRIBE, topic.c_str(), topic.length());
    m_subscribed.erase(iter);
    return true;
}

void MessagesClient::spin() {
    // do nothing now
    while (!m_exit_signal) {
        sleep_ms(1000);
    }
}

void MessagesClient::stop() {
    m_exit_signal = true;
    // unsubscribe all
    std::vector<std::string> topic_list;
    for (auto& sub : m_subscribed) {
        // do not erase map node in loop
        topic_list.push_back(sub.first);
    }
    for (auto& topic : topic_list) {
        this->unsubscribe(topic);
    }

    m_routing_client->stop();

    for (auto&& pub : m_pub_socket) {
        pub.second->close();
    }
    m_sub_socket->close();

    // join subscribing thread
    if (m_listen_thread.joinable()) {
        m_listen_thread.join();
    }
}

bool MessagesClient::connected() {
    return m_routing_client->connected();
}

bool MessagesClient::node_list(SubscribeNodeList& node_list) {
    return m_routing_client->node_list(node_list);
}

////////////////////////////////////////////////////////////////////////////////
// call by routing client event callback
void MessagesClient::publisher_online(const std::string& topic, const std::string& addr) {
    assert(topic != "");
    assert(addr != "");

    auto iter = m_sub_waiting.find(topic);
    if (iter != m_sub_waiting.end()) {
        // publisher online
        try {
            m_sub_socket->connect(addr);
        } catch (zmq::error_t& e) {
            DEBUG_ERROR(e.what());
        }
    }
}

// call by routing client event callback
void MessagesClient::publisher_offline(const std::string& topic, const std::string& addr) {
    assert(topic != "");
    assert(addr != "");

    auto iter = m_subscribed.find(topic);
    if (iter != m_subscribed.end()) {
        // publisher offline
        try {
            m_sub_socket->disconnect(addr);
        } catch (zmq::error_t& e) {
            DEBUG_ERROR(e.what());
        }
    }
}

bool MessagesClient::register_subscription(const std::string& topic, CallbackInterface callback_ptr) {
    assert(topic != "");

    // sync with listening thread
    std::lock_guard<std::mutex> lock(m_sub_mutex);
    // prepare callback first, open gate later
    auto iter = m_subscribed.find(topic);
    if (iter != m_subscribed.end()) {
        // already subscribed
        return false;
    }
    m_subscribed.insert(std::make_pair(topic, callback_ptr));

    // lookup and connect publisher socket addr
    try {
        std::vector<std::string> sub_addr_list;
        if (m_routing_client->lookup_sub_addr(topic, sub_addr_list)) {
            for (auto& addr : sub_addr_list) {
                m_sub_socket->connect(addr.c_str());
            }
        }
        m_sub_socket->setsockopt(ZMQ_SUBSCRIBE, topic.c_str(), topic.length());
    } catch (zmq::error_t& e) {
        DEBUG_ERROR(e.what());
        return false;
    }

    // waiting for other publishers with same topic
    m_sub_waiting.insert(topic);

    // start listen on first subscription
    if (!m_have_subscription) {
        m_listen_thread = std::thread(&MessagesClient::listen_thread, this);
        m_have_subscription = true;
    }

    return true;
}

void MessagesClient::listen_thread() {
    ipc::util::common::set_thread_name("iml-" + m_id);

    zmq::pollitem_t zmq_pool_item = {*m_sub_socket, 0, ZMQ_POLLIN, 0};
    while (!m_exit_signal) {
        try {
            int rc = zmq::poll(&zmq_pool_item, 1, m_pool_timeout);
            if (rc < 0) {
                continue;
            }

            if (zmq_pool_item.revents & ZMQ_POLLIN) {
                MessageEnvelope msg;
                auto iter = m_subscribed.end();

                // sync block
                {
                    // sync with subscribe/unsubscribe
                    std::lock_guard<std::mutex> lock(m_sub_mutex);

                    if (msg.recv(*m_sub_socket) == false) {
                        continue; // no more messages
                    }

                    // lookup callback entry
                    iter = m_subscribed.find(msg.topic());
                    if (iter == m_subscribed.end()) {
                        continue; // topic not subscribed
                    }
                }

                MessageHeader header;
                header.topic = msg.topic();
                header.publisher = msg.publisher();
                header.serial = msg.serial();
                header.send_time_ms = msg.send_time_ms();
                header.send_time_us = msg.send_time_us();
                header.recv_time_ms = msg.recv_time_ms();
                header.recv_time_us = msg.recv_time_us();

                // set param & check valid & invoke callback
                auto callback_handle = iter->second;
                if (m_callback_mode == CallbackMode::sync) {
                    // sync callback
                    callback_handle->call(msg.payload(), header);
                } else {
                    // async callback
                    auto callback_task = std::bind(&VirtualMessageCallback::call,
                        callback_handle, msg.payload(), header);
                    m_thread_pool->push("im-" + header.topic, callback_task);
                }
            }
        } catch (zmq::error_t& e) {
            DEBUG_ERROR(e.what());
        }
    }
}

std::mutex& MessagesClient::pub_mutex(const std::string& topic) {
    assert(topic != "");
    return m_pub_mutex[topic];
}

} // namespace messages
} // namespace util
} // namespace ipc
