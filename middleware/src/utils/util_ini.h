#pragma once
#include <string>
#include <map>

namespace ipc {
namespace util {

// section-name -> value
using IniMap = std::map<std::string, std::string>;

class IniParser {

public:
	IniParser() {};
	IniParser(const std::string& file);
    ~IniParser() {};

    IniMap get();
	void parse(const std::string& text);

private:
	IniMap m_ini;
};

} // namespace util
} // namespace ipc

