#pragma once
#include <string>
#include <fstream>
#include <google/protobuf/message.h>
#include <google/protobuf/text_format.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include "util_utils.h"

namespace ipc {
namespace util {

////////////////////////////////////////////////////////////////////////////////
// disabled
template<class T, class Enable = void>
class Serializer {
public:
    Serializer(){}
    ~Serializer(){}
};

////////////////////////////////////////////////////////////////////////////////
// for protobuf object
template<class T>
class Serializer<T, typename std::enable_if<std::is_base_of<::google::protobuf::Message, T>::value>::type> {
public:
    Serializer(){}
    ~Serializer(){}

    bool to_string(const T& t, std::string& buffer) {
        return t.SerializeToString(&buffer);
    }

    bool from_string(const std::string& buffer, T& t) {
        return t.ParseFromString(buffer);
    }
};

////////////////////////////////////////////////////////////////////////////////
// for std::string
template<>
class Serializer<std::string> {
public:
    Serializer(){}
    ~Serializer(){}

    bool to_string(const std::string& t, std::string& buffer) {
        buffer = t;
        return true;
    }

    bool from_string(const std::string& buffer, std::string& t) {
        t = buffer;
        return true;
    }
};

////////////////////////////////////////////////////////////////////////////////
// for int64_t
template<>
class Serializer<int64_t> {
public:
    Serializer(){}
    ~Serializer(){}

    bool to_string(const int64_t& t, std::string& buffer) {
        buffer = std::to_string(t);
        return true;
    }

    bool from_string(const std::string& buffer, int64_t& t) {
        t = safe_stoll(buffer);
        return true;
    }
};

////////////////////////////////////////////////////////////////////////////////
// for double
template<>
class Serializer<double> {
public:
    Serializer(){}
    ~Serializer(){}

    bool to_string(const double& t, std::string& buffer) {
        buffer = std::to_string(t);
        return true;
    }

    bool from_string(const std::string& buffer, double& t) {
        t = safe_stod(buffer);
        return true;
    }
};

////////////////////////////////////////////////////////////////////////////////
// parse prototxt file
template<typename T>
bool parse_prototxt(const std::string& file, T& prototxt) {
    assert(file != "");
    assert((std::is_base_of<::google::protobuf::Message, T>::value));

    bool res = false;
    std::ifstream fs(file, std::ios::in|std::ios::binary);
    if (fs.is_open()) {
        google::protobuf::io::IstreamInputStream is(&fs);
        res = google::protobuf::TextFormat::Parse(&is, &prototxt);
        fs.close();
    }
    return res;
}

// load prototxt config
template<typename T>
bool load_config(const std::string& file_name, T& config) {
    std::string config_path = safe_getenv(ENV_CONFIG_PATH);
    return parse_prototxt(config_path+"/"+file_name, config);
}

} // namespace util
} // namespace ipc