#include "game/map.h"

#include <algorithm>
#include <utility>

#include <stdexcept>

namespace game {
	auto get_tileset(map & map_data, tile::id id) -> tileset& {
		return const_cast<tileset&>(get_tileset(std::as_const(map_data), id));
	}
	auto get_tileset(map const& map_data, tile::id id) -> tileset const& {
		auto const it_tileset = std::find_if(map_data.tilesets.rbegin(), map_data.tilesets.rend(), [id] (tileset const& tileset) { return tileset.starting_id <= id; });
		if(it_tileset == map_data.tilesets.rend()) { throw std::runtime_error("Invalid tile id in game::get_tileset"); }
		return *it_tileset;
	}
}