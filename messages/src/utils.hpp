#pragma once
#include <string>
#include <fstream>
#include <google/protobuf/message.h>
#include <google/protobuf/text_format.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <glog/logging.h>

namespace ipc {
namespace messages {

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
static std::string basename(const std::string& filename) {
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
    return name;
}
////////////////////////////////////////////////////////////////////////////////
// 
// #include <glob.h>
// bool glob_files(const std::string& pattern, std::vector<std::string>& files) {
//     glob_t glob_result;
//     memset(&glob_result, 0, sizeof(glob_result));

//     int res = glob(pattern.c_str(), GLOB_TILDE, nullptr, &glob_result);
//     if (res != 0) {
//         globfree(&glob_result);
//         return false;
//     }

//     for (size_t i = 0; i < glob_result.gl_pathc; ++i) {
//         files.push_back(std::string(glob_result.gl_pathv[i]));
//     }

//     globfree(&glob_result);
//     return true;
// }

////////////////////////////////////////////////////////////////////////////////
// 

} //namespace messages
} //namespace ipc
