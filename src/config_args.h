#pragma once

#include <optional>
#include <string>
#include <iosfwd>

#include "serial/error.h"
#include <tl/expected.hpp>

struct config_args {
    struct resource {
        std::optional<std::string> path;
    } resource_config;
};

auto parse_config_args(std::istream & i) -> tl::expected<config_args, serial::error>;