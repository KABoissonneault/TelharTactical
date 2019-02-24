#pragma once

#include "game/map.h"

#include "serial/error.h"
#include "sdl/texture.h"

#include <tl/expected.hpp>
#include <iosfwd>

struct SDL_Renderer;

namespace serial {
    auto load_tiled_json(std::istream& map_data) -> tl::expected<game::map, error>;
    auto get_tiled_tileset_image(std::istream& tileset_data) -> tl::expected<std::string, error>;
}