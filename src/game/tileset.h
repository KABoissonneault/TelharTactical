#pragma once

#include "math/vector2.h"
#include "sdl/texture.h"

#include <vector>

namespace game {
    constexpr size_t tile_pixel_length = 32;
    
    struct tileset {
        sdl::texture texture;
    };

    inline auto get_row_length(tileset const& t) -> size_t { 
        return t.texture.get_dimensions().x / tile_pixel_length; 
    }

    using tileset_index = int;
}