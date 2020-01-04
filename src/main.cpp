#include "command_args.h"
#include "config_args.h"
#include "game_data.h"
#include "sdl/macro.h"

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <cstdio>
#include <fstream>

namespace {
	void print_args(int argc, char** argv) {
        for (int i = 0; i < argc; ++i) {
            std::printf("Arg %d: %s\n", i, argv[i]);
        }
	}

    auto get_config_args() -> config_args {
        std::ifstream config_file("config.ini");
        if (!config_file.is_open()) {
            std::printf("No config file detected.\n");
            return {};
        }
        auto result = parse_config_args(config_file);
        if (!result) {
            throw std::runtime_error(fmt::format("Invalid config file: {}", result.error().description));
        }
        return std::move(*result);
    }

    void print_video_drivers() {
        int const num_video_drivers = SDL_GetNumVideoDrivers();
        if (num_video_drivers < 0) {
            std::printf("SDL Video Drivers could not be queried: %s", SDL_GetError());
            return;
        }
        std::printf("Video drivers (current: '%s'):\n", SDL_GetCurrentVideoDriver());
        for (int i = 0; i < num_video_drivers; ++i) {
            std::printf("\t(%d) %s\n", i, SDL_GetVideoDriver(i));
        }
    }
	
    void run_game(int argc, char** argv) {
        {
            SDL_version compiled_version, linked_version;

            SDL_VERSION(&compiled_version);
            SDL_GetVersion(&linked_version);
            fmt::print("SDL version: compiled={}.{}.{}, linked={}.{}.{}\n",
                compiled_version.major, compiled_version.minor, compiled_version.patch,
                linked_version.major, linked_version.minor, linked_version.patch
            );
        }
		
        KT_SDL_ENSURE(SDL_Init(SDL_INIT_VIDEO));
        auto const sdl_destroy = gsl::finally(&SDL_Quit);

        {
            SDL_version compiled_version;
            SDL_IMAGE_VERSION(&compiled_version);
            SDL_version const* linked_version = IMG_Linked_Version();
            fmt::print("SDL_image version: compiled={}.{}.{}, linked={}.{}.{}\n",
                compiled_version.major, compiled_version.minor, compiled_version.patch,
                linked_version->major, linked_version->minor, linked_version->patch
            );
        }
		
        KT_SDL_ENSURE(IMG_Init(IMG_INIT_PNG));
        auto const img_destroy = gsl::finally(&IMG_Quit);

        {
            SDL_version compiled_version;
            SDL_TTF_VERSION(&compiled_version);
            SDL_version const* linked_version = TTF_Linked_Version();
            fmt::print("SDL_ttf version: compiled={}.{}.{}, linked={}.{}.{}\n",
                compiled_version.major, compiled_version.minor, compiled_version.patch,
                linked_version->major, linked_version->minor, linked_version->patch
            );
        }
		
        KT_SDL_ENSURE(TTF_Init());
        auto const ttf_destroy = gsl::finally(&TTF_Quit);

        config_args conf = get_config_args();
        command_args const cmd = parse_args({ argv, argc });

        if (cmd.print_video_drivers) {
            print_video_drivers();
        }
		
        game_data game{
            cmd,
            std::move(conf)
        };

        game.run();
    }
}

int main(int argc, char** argv) try {
    print_args(argc, argv);

    run_game(argc, argv);   

    return EXIT_SUCCESS;
} catch(std::exception const& e) {
    std::printf("Uncaught exception: %s\n", e.what());
    return EXIT_FAILURE;
}