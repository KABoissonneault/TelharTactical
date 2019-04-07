#pragma once

#include "game/tile.h"

#include <vector>
#include <variant>

namespace game {
    struct layer {
        enum class id_t {};
        enum class type { tile };
        
        id_t id;
        
        struct tile_data {
            std::vector<tile_chunk> chunks;
        };

        std::variant<tile_data> data;

        auto get_type() const noexcept -> type {
            static_assert(type::tile == type{0}, "Cast hack over here :D");
            return static_cast<type>(data.index());
        }
    };

    
}