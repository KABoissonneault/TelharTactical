#pragma once

#include "game/tile.h"
#include "game/object.h"

#include <vector>
#include <variant>

namespace game {
    struct layer {
        enum class id_t {};
        enum class type { tile, object };
        
        id_t id;
        
        struct tile_data {
            std::vector<tile_chunk> chunks;
        };

    	struct object_data {
            std::vector<object> objects;
    	};

        std::variant<tile_data, object_data> data;

        auto get_type() const noexcept -> type {
            static_assert(type::tile == type{0}, "Cast hack over here :D");
            return static_cast<type>(data.index());
        }
    };

    
}