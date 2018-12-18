#include "sdl/resource.h"
#include "sdl/macro.h"

#include <SDL.h>

#include <optional>
#include <gsl/span>
#include <gsl/gsl_util>
#include <string_view>
#include <cstdio>

#include <string>
using namespace std::string_literals;

struct vector2i {
    int x, y;
};

struct command_args {
    std::optional<vector2i> window_size;
};

auto parse_args(gsl::span<char const* const> args) -> command_args {
    command_args result;
    for(int i = 0; i < args.size(); ++i) {
        std::string_view const arg = args[i];
        if(arg == "--window_size") {
            if(i + 2 >= args.size() || args[i+1][0] == '-' || args[i + 2][0] == '-') {
                throw std::invalid_argument("Missing arguments after '--window_size'");
            }

            char* arg_end;
            int const x = std::strtol(args[i + 1], &arg_end, 10);
            if(arg_end == args[i + 1]) {
                throw std::invalid_argument("First argument to '--window_size' was not an integer: "s + args[i + 1]);
            }
            int const y = std::strtol(args[i + 2], &arg_end, 10);
            if(arg_end == args[i + 2]) {
                throw std::invalid_argument("Second argument to '--window_size' was not an integer: "s + args[i + 2]);
            }
            result.window_size = vector2i{x, y};
            i += 2;
        }
    }
    return result;
}

struct sdl_video_resources {
    sdl::unique_window window;
    sdl::unique_renderer renderer;
};

auto initialize_sdl_video(command_args const& args) -> sdl_video_resources {
    vector2i const window_size = args.window_size.value_or(vector2i{1280, 720});
    SDL_Window* window;
    SDL_Renderer* renderer;
    KT_SDL_ENSURE(SDL_CreateWindowAndRenderer(window_size.x, window_size.y, 0, &window, &renderer));
    return {sdl::unique_window(window), sdl::unique_renderer(renderer)};
}

void run_loop(sdl_video_resources video) {
    bool quit = false;
    while(!quit) {
        SDL_Event e;
        while(SDL_PollEvent(&e)) {
            switch(e.type) {
                case SDL_QUIT:
                    quit = true;
                    break;
            }
        }
        KT_SDL_ENSURE(SDL_RenderClear(video.renderer.get()));
    }
}

int main(int argc, char** argv) try {
    for(int i = 0; i < argc; ++i) {
        std::printf("Arg %d: %s\n", i, argv[i]);
    }

    command_args const args = parse_args({argv, argc});

    KT_SDL_ENSURE(SDL_Init(SDL_INIT_VIDEO));
    auto const sdl_destroy = gsl::finally(&SDL_Quit);

    run_loop(initialize_sdl_video(args));

    return EXIT_SUCCESS;
} catch(std::exception const& e) {
    std::printf("Uncaught exception: %s\n", e.what());
    return EXIT_FAILURE;
}