#include <cassert>
#include <sstream>
#include "util_string.h"
#include "util_debug.h"
#include "util_ini.h"

namespace ipc {
namespace util {
namespace common {

IniParser::IniParser(const std::string& file) {
    assert(file != "");
    std::string text;
    if (string_load(file, text)) {
        this->parse(text);
    }
}

IniMap IniParser::get() {
    return m_ini;
}

void IniParser::parse(const std::string& text) {
    assert(text != "");

    std::stringstream ss(text);
    std::string curr_section = "";
    std::string line;

    while (std::getline(ss, line, '\n')) {
        line = string_trim(line);

        if (line == "") {
            continue;
        }

        // comment
        if ((line[0] == '#') || (line[0] == ';')) {
            continue;
        }

        // new section
        if ((line[0] == '[') && (line[line.length() - 1] == ']')) {
            line = string_trim(line.substr(1, line.length() - 2));
            if (line != "") {
                curr_section = line + ".";
            }
            continue;
        }

        auto pos = line.find("=");

        // unknown
        if (pos == line.npos) {
            continue;
        }

        // name=value
        std::string name = string_trim(line.substr(0, pos));
        std::string value = string_trim(line.substr(pos + 1));
        if ((name == "") || (value == "")) {
            // invalid
            continue;
        }

        // section-name = value
        m_ini[curr_section + name] = value;
    }
}

} // namespace common
} // namespace util
} // namespace ipc
