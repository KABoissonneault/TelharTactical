#include "sdl/resource.h"
#include "sdl/macro.h"

#include "command_args.h"
#include "config_args.h"

#include "game/map.h"
#include "serial/tiled.h"

#include <SDL.h>
#include <SDL_image.h>

#include <fmt/format.h>
#include <FMT/ostream.h>

#include <gsl/gsl_util>
#include <cstdio>
#include <fstream>
#include <map>
#include <filesystem>

namespace {
    struct program_args {
        command_args cmd;
        config_args cfg;
    };

    auto get_resource_path(program_args const& args) -> std::filesystem::path {
        return args.cfg.resource_config.path.value_or("res");
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
    };

    auto load_game_data(program_args const& args) -> game_data {
        std::ifstream map(get_resource_path(args) / "test_infinite_map.json");
        if(!map) {
            throw std::runtime_error("Could not open 'res/test_infinite_map.json'");
        }

        auto map_result = serial::load_tiled_json(map);
        if(!map_result) {
            throw std::runtime_error(fmt::format("Failed to load 'res/test_infinite_map.json' Tiled map: {}", map_result.error().description));
        }

        return game_data{*std::move(map_result)};
    }

    auto load_tilesets(program_args const& args, gsl::span<game::tileset const> tilesets, SDL_Renderer & renderer) -> std::vector<sdl::texture> {
        auto const resource_path = get_resource_path(args);
        std::vector<sdl::texture> textures;
        for(game::tileset const& tileset : tilesets) {
            auto const tiled_tileset = resource_path / tileset.source;
            auto tiled_data = std::ifstream(tiled_tileset);
            if(!tiled_data) {
                throw std::runtime_error(fmt::format("Failed to load Tiled tileset '{}'", tiled_tileset));
            }
            auto const result = serial::get_tiled_tileset_image(tiled_data);
            if(!result) {
                throw std::runtime_error(fmt::format("Could not find image data in Tiled tileset '{}': {}", tiled_tileset, result.error().description));
            }
            auto const& texture_path = *result;
            auto const path = resource_path / texture_path;
            auto const texture = IMG_LoadTexture(&renderer, path.string().c_str());
            if(texture == nullptr) {
                throw std::runtime_error(fmt::format("Failed to load tileset '{}'", path));
            }
            textures.emplace_back(texture);
        }
        return textures;
    }

    bool is_quit_event(SDL_Event const& e) {
        return e.type == SDL_QUIT || e.type == SDL_KEYUP && e.key.keysym.sym == SDLK_ESCAPE;
    }

    void render_tile_layer(game::layer::tile_data const& tiles, sdl::texture & tileset, SDL_Renderer & renderer) {
        for(game::tile_chunk const& chunk : tiles.chunks) {
            auto const chunk_screen_position = element_multiply(chunk.position, game::tile::dimensions);
            for(size_t tile_index = 0; tile_index < chunk.tiles.size(); ++tile_index) {
                game::tile const tile = chunk.tiles[tile_index];
                if(tile.data == game::tile::id::none) {
                    continue;
                }

                auto const tileset_key = static_cast<int>(tile.data) - 1; // first real tile id is at 1
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

    void run_loop(program_args const& args) {
        // Init
        sdl_video_resources const video = initialize_sdl_video(args);
        auto const renderer = video.renderer.get();
        game_data game = load_game_data(args);
        auto tilesets = load_tilesets(args, game.map.tilesets, *video.renderer);

        bool quit = false;
        while(!quit) {
            // Event
            SDL_Event e;
            while(SDL_PollEvent(&e)) {
                if(is_quit_event(e)) {
                    quit = true;
                }
            }

            // Update

            // Render
            KT_SDL_ENSURE(SDL_RenderClear(renderer));

            for(game::layer const& layer : game.map.layers) {
                if(layer.get_type() == game::layer::type::tile) {
                    render_tile_layer(std::get<game::layer::tile_data>(layer.data), tilesets.front(), *renderer);
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

    program_args const args{
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

    run_loop(args);

    return EXIT_SUCCESS;
} catch(std::exception const& e) {
    std::printf("Uncaught exception: %s\n", e.what());
    return EXIT_FAILURE;
}