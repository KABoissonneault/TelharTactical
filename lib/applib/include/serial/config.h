#pragma once

#include <iosfwd>
#include <string_view>

#include "serial/error.h"

#include <tl/expected.hpp>

namespace serial {
    using ini_handler = void(*)(std::string_view section, std::string_view key, std::string_view value, void* user_data);
    // The handler is called on every property
    auto parse_config(std::istream & file, ini_handler handler, void* user_data) -> tl::expected<void, error>;
}