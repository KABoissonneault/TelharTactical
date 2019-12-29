#include "command_args.h"
#include "config_args.h"
#include "game_data.h"
#include "sdl/macro.h"

#include <SDL.h>
#include <SDL_image.h>

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <cstdio>
#include <fstream>

int main(int argc, char** argv) try {
    for(int i = 0; i < argc; ++i) {
        std::printf("Arg %d: %s\n", i, argv[i]);
    }

	KT_SDL_ENSURE(SDL_Init(SDL_INIT_VIDEO));
	auto const sdl_destroy = gsl::finally(&SDL_Quit);

	KT_SDL_ENSURE(IMG_Init(IMG_INIT_PNG));
	auto const img_destroy = gsl::finally(&IMG_Quit);

    config_args conf = []() -> config_args {
        std::ifstream config_file("config.ini");
        if (!config_file.is_open()) {
            return {};
        }
        auto result = parse_config_args(config_file);
        if (!result) {
            throw std::runtime_error(fmt::format("Invalid config file: {}", result.error().description));
        }
        return std::move(*result);
    }();
	
    game_data game{
        parse_args({argv, argc}),
        std::move(conf)
    };

	game.run();

    return EXIT_SUCCESS;
} catch(std::exception const& e) {
    std::printf("Uncaught exception: %s\n", e.what());
    return EXIT_FAILURE;
}