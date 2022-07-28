#ifndef RECORDER_MESSAGES_NODE_H
#define RECORDER_MESSAGES_NODE_H

#include "messages_factory.h"
#include "pavaro/pavaro.h"
#include <map>
#include <set>
#include <stdint.h>

namespace ipc {
namespace messages {

class MessageNodeManager {
public:
    static MessageNodeManager& content() {
        static MessageNodeManager s_node;
        return s_node;
    }

    IClientNode* getnode(const std::string& url) {
        /*
        auto iter = _node_map.find(url);
        if (iter != _node_map.end()) {
            return iter->second;
        }
        */
        std::string type;
        std::string ip;
        uint16_t port = 0;
        if (decodeurl(url, type, ip, port) == false) {
            ipc::log_error("decode url error", url);
            return nullptr;
        }
        std::stringstream ss;
        ss << ip << ":" << port;
        /*
        auto iter2 = _address_set.find(ss.str());
        if (iter2 != _address_set.end()) {
            ipc::log_error("address is already in use", url);
            return nullptr;
        }
        */
        MessageFactory factory(type);
        IClientNode* node = factory.create_client(url, ip, port);
        if (node == nullptr) {
            ipc::log_error("create client node error", url);
            return nullptr;
        }
        _node_list.push_back(node);
        //_address_set.insert(ss.str());
        //_node_map[url] = node;
        return node;
    }

    bool decodeurl(const std::string&url, std::string& type, std::string& ip, uint16_t& port) {
        size_t colon = url.find(':');
        if (colon != std::string::npos)
        {
            std::string temp = url.substr(0, colon);
            port = static_cast<uint16_t>(atoi(url.c_str() + colon + 1));

            size_t col = temp.find('@');
            if (col !=std::string::npos) {
                type = temp.substr(0, col);
                ip = temp.substr(col + 1, temp.size());
            } else {
                return false;
            }
        } else {
            return false;
        }
        ipc::log_info("decode", url, "type:", type, "ip:", ip, "port:", port);
        return true;
    }

    void destroy() {
        for (auto && node : _node_list) {
            delete node;
        }
    }
private:
    MessageNodeManager() {}
    ~MessageNodeManager() {
        /*
        for (auto&& node : _node_map) {
            delete node.second;
        }
        */
    }
    std::vector<IClientNode*> _node_list;
    //std::map<std::string, IClientNode*> _node_map;
    //std::set<std::string> _address_set;
};

}
}
#endif //RECORDER_MESSAGES_NODE_H
