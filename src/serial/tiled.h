#pragma once

#include "game/map.h"

#include "serial/error.h"

#include <tl/expected.hpp>
#include <iosfwd>

namespace serial {
    auto load_tiled_json(std::istream& map_data) -> tl::expected<game::map, error>;
}