#pragma once

#include "layer.h"

#include <string>

namespace game {
    struct tileset {
        std::string source;
		tile::id starting_id;
    };

    struct map {
        std::vector<layer> layers;
        std::vector<tileset> tilesets;
    };
	
	// tile id should be greater than 0
	auto get_tileset(map & map_data, tile::id id) -> tileset&;
	auto get_tileset(map const& map_data, tile::id id) -> tileset const&;
}