#pragma once

#include "tileset.h"

#include <array>

namespace game {
    constexpr size_t chunk_length = 4;

    struct tile {
        tileset_index index;
    };

    struct tile_chunk {
        std::array<std::array<tile, chunk_length>, chunk_length> tiles;
    };
}