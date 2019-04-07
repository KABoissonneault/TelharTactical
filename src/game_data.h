#pragma once

#include "command_args.h"
#include "config_args.h"

#include "game/map.h"
#include "sdl/texture.h"
#include "sdl/resource.h"
#include "math/vector2.h"

#include <map>

class game_data {
public:
	game_data(command_args cmd, config_args cfg);

	void run();

private:
	command_args cmd;
	config_args cfg;
	sdl::unique_window window;
	sdl::unique_renderer renderer;
	game::map map;
	std::map<std::string, sdl::texture> texture_bank;
	math::vector2i screen_pixel_offset{0, 0};

	void render_tile_layer(game::layer::tile_data const& tiles);
};