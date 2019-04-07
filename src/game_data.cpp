#include "game_data.h"

#include <filesystem>
#include <string_view>
#include <fstream>

#include <SDL_video.h>
#include <SDL_render.h>
#include <SDL_events.h>
#include <SDL_image.h>

#include <fmt/format.h>
#include <fmt/ostream.h>

#include "sdl/resource.h"
#include "sdl/macro.h"
#include "serial/tiled.h"

#include "algorithm_extra.h"

namespace {
	constexpr std::string_view resource_section = "resource";
	constexpr std::string_view path_key = "path";

	constexpr std::string_view game_section = "game";
	constexpr std::string_view default_map_key = "default_map";

	auto get_resource_path(config_args const& cfg) -> std::filesystem::path {
		auto const path = cfg.get_value(resource_section, path_key).value_or("res");
		return {path.begin(), path.end()};
	}
	
	void print_video_drivers() {
		int const num_video_drivers = SDL_GetNumVideoDrivers();
		if(num_video_drivers < 0) {
			std::printf("SDL Video Drivers could not be queried: %s", SDL_GetError());
			return;
		}
		std::printf("Video drivers (current: '%s'):\n", SDL_GetCurrentVideoDriver());
		for(int i = 0; i < num_video_drivers; ++i) {
			std::printf("\t(%d) %s\n", i, SDL_GetVideoDriver(i));
		}
	}

	auto create_window(command_args const& cmd) -> sdl::unique_window {
		auto const window_size = cmd.window_size.value_or(math::vector2i{1280, 720});
		return sdl::unique_window(SDL_CreateWindow("TelharTactical", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, window_size.x, window_size.y, SDL_WINDOW_RESIZABLE));
	}
	
	auto create_renderer(SDL_Window & window) -> sdl::unique_renderer {
		return sdl::unique_renderer(SDL_CreateRenderer(&window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC));
	}

	bool is_quit_event(SDL_Event const& e) {
		return e.type == SDL_QUIT || e.type == SDL_KEYUP && e.key.keysym.sym == SDLK_ESCAPE;
	}

	auto load_map(config_args const& cfg, std::string_view map_name) -> game::map {
		auto const path = get_resource_path(cfg).append(map_name.begin(), map_name.end());
		std::ifstream map_data(path);
		if(!map_data) {
			throw std::runtime_error(fmt::format("Could not open '{}'", path));
		}

		auto map_result = serial::load_tiled_json(map_data);
		if(!map_result) {
			throw std::runtime_error(fmt::format("Failed to load '{}' Tiled map: {}", map_name, map_result.error().description));
		}

		return *std::move(map_result);
	}

	auto load_default_map(config_args const& cfg) -> game::map {
		auto const default_map = cfg.get_value(game_section, default_map_key);
		if(default_map) {
			return load_map(cfg, *default_map);
		} else {
			return {};
		}
	}

	auto load_texture_bank(config_args const& cfg, gsl::span<game::tileset const> tilesets, SDL_Renderer & renderer, std::map<std::string, sdl::texture> texture_bank = {}) 
		-> std::map<std::string, sdl::texture> {
		// Clear textures not requested
		erase_if(texture_bank, [tilesets] (auto const& texture_data) -> bool {
			return std::find_if(tilesets.begin(), tilesets.end(), [&texture_data] (game::tileset const& tileset) -> bool {
				return tileset.source == texture_data.first;
			}) == tilesets.end();
		});

		for(auto const& tileset : tilesets) {
			if(texture_bank.find(tileset.source) != texture_bank.end()) {
				continue;
			}

			auto const resource_path = get_resource_path(cfg);
			auto const tiled_tileset = resource_path / tileset.source;
			auto tiled_data = std::ifstream(tiled_tileset);
			if(!tiled_data) {
				throw std::runtime_error(fmt::format("Failed to load Tiled tileset '{}'", tiled_tileset));
			}

			auto const result = serial::get_tiled_tileset_image(tiled_data);
			if(!result) {
				throw std::runtime_error(fmt::format("Could not find image data in Tiled tileset '{}': {}", tiled_tileset, result.error().description));
			}
			auto const& texture_name = *result;

			auto const texture_path = resource_path / texture_name;
			auto const texture = IMG_LoadTexture(&renderer, texture_path.string().c_str());
			if(texture == nullptr) {
				throw std::runtime_error(fmt::format("Failed to load tileset '{}'", texture_path));
			}
			texture_bank.emplace(tileset.source, texture);
		}

		return texture_bank;
	}
}

game_data::game_data(command_args cmd, config_args cfg)
	: cmd(std::move(cmd))
	, cfg(std::move(cfg))
	, window(create_window(this->cmd))
	, renderer(create_renderer(*window))
	, map(load_default_map(this->cfg))
	, texture_bank(load_texture_bank(this->cfg, map.tilesets, *renderer)) {
	if(this->cmd.print_video_drivers) {
		print_video_drivers();
	}
}

void game_data::run() {
	bool quit = false;
	while(!quit) {
		// Event
		SDL_Event e;
		while(SDL_PollEvent(&e)) {
			if(is_quit_event(e)) {
				quit = true;
			} else if(e.type == SDL_KEYUP && e.key.keysym.sym == SDLK_F2) {
				map = load_default_map(cfg);
				texture_bank = load_texture_bank(cfg, map.tilesets, *renderer, std::move(texture_bank));
			} else if(e.type == SDL_KEYUP && e.key.keysym.sym == SDLK_LEFT) {
				screen_pixel_offset += math::vector2i{game::tile::dimensions.x, 0};
			} else if(e.type == SDL_KEYUP && e.key.keysym.sym == SDLK_RIGHT) {
				screen_pixel_offset += math::vector2i{-game::tile::dimensions.x, 0};
			} else if(e.type == SDL_KEYUP && e.key.keysym.sym == SDLK_UP) {
				screen_pixel_offset += math::vector2i{0, game::tile::dimensions.y};
			} else if(e.type == SDL_KEYUP && e.key.keysym.sym == SDLK_DOWN) {
				screen_pixel_offset += math::vector2i{0, -game::tile::dimensions.y};
			}
		}

		// Update

		// Render
		KT_SDL_ENSURE(SDL_RenderClear(renderer.get()));


		for(game::layer const& layer : map.layers) {
			if(layer.get_type() == game::layer::type::tile) {
				render_tile_layer(std::get<game::layer::tile_data>(layer.data));
			}
		}

		SDL_RenderPresent(renderer.get());
	}
}

void game_data::render_tile_layer(game::layer::tile_data const& tiles) {
	for(game::tile_chunk const& chunk : tiles.chunks) {
		auto const chunk_screen_position = element_multiply(chunk.position, game::tile::dimensions);
		for(size_t tile_index = 0; tile_index < chunk.tiles.size(); ++tile_index) {
			game::tile const tile = chunk.tiles[tile_index];
			if(tile.data == game::tile::id::none) {
				continue;
			}

			game::tileset const& tileset_source = game::get_tileset(map, tile.data);
			auto const tileset_key = static_cast<int>(tile.data) - static_cast<int>(tileset_source.starting_id);

			auto const it_tileset = texture_bank.find(tileset_source.source);
			if(it_tileset == texture_bank.end()) throw std::runtime_error(fmt::format("Non-loaded texture '{}'", tileset_source.source));
			sdl::texture & tileset = it_tileset->second;

			auto const tileset_tile_width = tileset.get_dimensions().x / game::tile::dimensions.x;
			auto const tileset_coords = element_multiply(math::vector2i{tileset_key % tileset_tile_width, tileset_key / tileset_tile_width}, game::tile::dimensions);

			auto const chunk_coords = math::vector2i{static_cast<int>(tile_index) % game::tile_chunk::dimensions.x, static_cast<int>(tile_index) / game::tile_chunk::dimensions.x};
			auto const screen_coords = screen_pixel_offset + chunk_screen_position + element_multiply(chunk_coords, game::tile::dimensions);

			SDL_Rect const texture_rect{
				tileset_coords.x,
				tileset_coords.y,
				game::tile::dimensions.x,
				game::tile::dimensions.y
			};
			SDL_Rect const screen_rect{
				screen_coords.x,
				screen_coords.y,
				game::tile::dimensions.x,
				game::tile::dimensions.y
			};
			SDL_RenderCopy(renderer.get(), tileset.get_texture(), &texture_rect, &screen_rect);
		}
	}
}
