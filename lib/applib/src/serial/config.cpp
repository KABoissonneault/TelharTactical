#include "serial/config.h"

#include <string>

#include <fmt/format.h>

#include <cctype>

namespace serial {
    namespace {
        template<typename StringT>
        auto invalid_argument(StringT&& str) {
            return tl::make_unexpected(error{std::make_error_code(std::errc::invalid_argument), std::forward<StringT>(str)});
        }
    }
    auto parse_config(std::istream & file, ini_handler handler, void* user_data) -> tl::expected<void, error> {
        std::string section;
        std::string line;
        int line_count = 0;
        while(std::getline(file, line)) {
            ++line_count;
            auto const line_view = std::string_view(line);
            if(line_view.empty() || line_view[0] == ';') {
                continue;
            } else if(line_view[0] == '[') {
                section = line_view.substr(1, line.size() - 2);
            } else {
                auto const it_key_end = std::find(line_view.begin(), line_view.end(), '=');
                if(it_key_end == line_view.end()) {
                    return invalid_argument(fmt::format("Error on line {}: invalid property", line_count));
                }
                std::string_view const key(line_view.data(), std::distance(line_view.begin(), it_key_end));
                std::string_view const value(&*(it_key_end + 1), std::distance(it_key_end + 1, line_view.end()));
                handler(section, key, value, user_data);
            }
        }
        return {};
    }
}