#pragma once

#include <iosfwd>

#include <gsl/string_span>

#include "serial/error.h"

#include <tl/expected.hpp>

namespace serial {
    using ini_handler = void(*)(gsl::cstring_span<> section, gsl::cstring_span<> key, gsl::cstring_span<> value, void* user_data);
    // The handler is called on every property
    auto parse_config(std::istream & file, ini_handler handler, void* user_data) -> tl::expected<void, error>;
}