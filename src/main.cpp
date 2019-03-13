#include "sdl/resource.h"
#include "sdl/macro.h"

#include "command_args.h"
#include "config_args.h"

#include "game/map.h"
#include "serial/tiled.h"

#include <SDL.h>
#include <SDL_image.h>

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <gsl/gsl_util>
#include <gsl/string_span>
#include <cstdio>
#include <fstream>
#include <map>
#include <filesystem>
#include "algorithm_extra.h"

namespace gsl {
	std::ostream& operator<<(std::ostream & o, gsl::cstring_span<> s) {
		return o.write(s.data(), s.size());
	}
}

namespace {
    struct program_args {
        command_args cmd;
        config_args cfg;
    };

	gsl::cstring_span<> const resource_section = "resource";
	gsl::cstring_span<> const path_key = "path";

	gsl::cstring_span<> const game_section = "game";
	gsl::cstring_span<> const default_map_key = "default_map";

    auto get_resource_path(program_args const& args) -> std::filesystem::path {
        auto const path = args.cfg.get_value(resource_section, path_key).value_or("res");
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

    struct sdl_video_resources {
        sdl::unique_window window;
        sdl::unique_renderer renderer;
    };

    auto initialize_sdl_video(program_args const& args) -> sdl_video_resources {
        auto const window_size = args.cmd.window_size.value_or(math::vector2i{1280, 720});
        SDL_Window* window;
        SDL_Renderer* renderer;
        KT_SDL_ENSURE(SDL_CreateWindowAndRenderer(window_size.x, window_size.y, SDL_WINDOW_RESIZABLE, &window, &renderer));
        return {sdl::unique_window(window), sdl::unique_renderer(renderer)};
    }

    struct game_data {
        game::map map;
		std::map<std::string, sdl::texture> texture_bank;
		program_args args;

		void load_map(gsl::cstring_span<> map_name) {
			auto const path = get_resource_path(args).append(map_name.begin(), map_name.end());
			std::ifstream map_data(path);
			if(!map_data) {
				throw std::runtime_error(fmt::format("Could not open '{}'", path));
			}

			auto map_result = serial::load_tiled_json(map_data);
			if(!map_result) {
				throw std::runtime_error(fmt::format("Failed to load '{}' Tiled map: {}", map_name, map_result.error().description));
			}

			map = *std::move(map_result);
		}

		void load_tilesets(gsl::span<game::tileset const> tilesets, SDL_Renderer & renderer) {
			for(auto const& tileset : tilesets) {
				if(texture_bank.find(tileset.source) != texture_bank.end()) {
					continue;
				}

				auto const resource_path = get_resource_path(args);
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

			// Clear textures not requested
			erase_if(texture_bank, [tilesets] (auto const& texture_data) -> bool {
				return std::find_if(tilesets.begin(), tilesets.end(), [&texture_data] (game::tileset const& tileset) -> bool { 
					return tileset.source == texture_data.first; 
				}) == tilesets.end();
			});
		}

		void render_tile_layer(game::layer::tile_data const& tiles, SDL_Renderer & renderer) {
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
					auto const screen_coords = chunk_screen_position + element_multiply(chunk_coords, game::tile::dimensions);

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
					SDL_RenderCopy(&renderer, tileset.get_texture(), &texture_rect, &screen_rect);
				}
			}
		}
    };   

    bool is_quit_event(SDL_Event const& e) {
        return e.type == SDL_QUIT || e.type == SDL_KEYUP && e.key.keysym.sym == SDLK_ESCAPE;
    }
	    
    void run_loop(program_args args) {
        // Init
		sdl_video_resources const video = initialize_sdl_video(args);
		auto const renderer = video.renderer.get();
		
		game_data game;
		game.args = std::move(args);

		auto const default_map = game.args.cfg.get_value(game_section, default_map_key);
		if(default_map) {
			game.load_map(*default_map);
			game.load_tilesets(game.map.tilesets, *renderer);
		}

        bool quit = false;
        while(!quit) {
            // Event
            SDL_Event e;
            while(SDL_PollEvent(&e)) {
                if(is_quit_event(e)) {
                    quit = true;
				} else if(e.type == SDL_KEYUP && e.key.keysym.sym == SDLK_F2) {
					game.load_map(*default_map);
					game.load_tilesets(game.map.tilesets, *renderer);
				}
            }

            // Update

            // Render
            KT_SDL_ENSURE(SDL_RenderClear(renderer));


            for(game::layer const& layer : game.map.layers) {
                if(layer.get_type() == game::layer::type::tile) {
                    game.render_tile_layer(std::get<game::layer::tile_data>(layer.data), *renderer);
                }
            }

            SDL_RenderPresent(renderer);
        }
    }
}

int main(int argc, char** argv) try {
    for(int i = 0; i < argc; ++i) {
        std::printf("Arg %d: %s\n", i, argv[i]);
    }

    program_args args{
        parse_args({argv, argc}),
        [] () -> config_args {
            std::ifstream config_file("config.ini");
            if(!config_file.is_open()) {
                return {};
            }
            auto result = parse_config_args(config_file);
            if(!result) {
                throw std::runtime_error(fmt::format("Invalid config file: {}", result.error().description));
            }
            return std::move(*result);
        }()
    };

    KT_SDL_ENSURE(SDL_Init(SDL_INIT_VIDEO));
    auto const sdl_destroy = gsl::finally(&SDL_Quit);

    KT_SDL_ENSURE(IMG_Init(IMG_INIT_PNG));
    auto const img_destroy = gsl::finally(&IMG_Quit);

    if(args.cmd.print_video_drivers) {
        print_video_drivers();
    }

    run_loop(std::move(args));

    return EXIT_SUCCESS;
} catch(std::exception const& e) {
    std::printf("Uncaught exception: %s\n", e.what());
    return EXIT_FAILURE;
}