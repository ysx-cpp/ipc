#pragma once
#include <string>
#include <fstream>
#include <google/protobuf/message.h>
#include <google/protobuf/text_format.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <glog/logging.h>

#define LOGINFO LOG(INFO)<<__FUNCTION__<<"|"
#define LOGERROR LOG(ERROR)<<__FUNCTION__<<"|"

////////////////////////////////////////////////////////////////////////////////
// parse prototxt file
template<typename T>
bool ParsePbtxt(const std::string& file, T& prototxt) {
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

////////////////////////////////////////////////////////////////////////////////
// load prototxt config
template<typename T>
bool LoadConfig(const std::string& file_name, T& config) {
    return ParsePbtxt(file_name, config);
}

////////////////////////////////////////////////////////////////////////////////
// 