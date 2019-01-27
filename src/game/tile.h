#pragma once

#include <vector>

#include "math/vector2.h"

namespace game {
    struct tile {
        static constexpr math::vector2i dimensions{32, 32}; // pixels
        enum class id {};
        id data;
    };

    struct tile_chunk {
        static constexpr math::vector2i dimensions{16, 16}; // tiles

        math::vector2i position;
        std::vector<tile> tiles;
    };
}