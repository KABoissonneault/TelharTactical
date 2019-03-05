#pragma once

#include <optional>
#include <gsl/span>

#include "math/vector2.h"

struct command_args {
    std::optional<math::vector2i> window_size;
    bool print_video_drivers = false;
};

auto parse_args(gsl::span<char const* const> args) -> command_args;