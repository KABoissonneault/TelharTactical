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

struct displacement {
    int x, y;
};

struct command_args {
    std::optional<displacement> window_size;
    bool print_video_drivers = false;
};

auto parse_window_size(gsl::span<char const* const> args) -> displacement {
    if(args.size() < 2 || args[0][0] == '-' || args[1][0] == '-') {
        throw std::invalid_argument("Missing arguments after '--window_size'");
    }

    char* arg_end;
    int const x = std::strtol(args[0], &arg_end, 10);
    if(arg_end == args[0]) {
        throw std::invalid_argument("First argument to '--window_size' was not an integer: "s + args[0]);
    }
    int const y = std::strtol(args[1], &arg_end, 10);
    if(arg_end == args[1]) {
        throw std::invalid_argument("Second argument to '--window_size' was not an integer: "s + args[1]);
    }

    return {x, y};
}

auto parse_args(gsl::span<char const* const> args) -> command_args {
    command_args result;
    for(int i = 0; i < args.size(); ++i) {
        std::string_view const arg = args[i];
        if(arg == "--window_size") {
            result.window_size = parse_window_size(args.subspan(i + 1));
            i += 2;
        } else if(arg == "--print-video-drivers") {
            result.print_video_drivers = true;
        }
    }
    return result;
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

auto initialize_sdl_video(command_args const& args) -> sdl_video_resources {
    displacement const window_size = args.window_size.value_or(displacement{1280, 720});
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

    if(args.print_video_drivers) {
        print_video_drivers();
    }

    run_loop(initialize_sdl_video(args));

    return EXIT_SUCCESS;
} catch(std::exception const& e) {
    std::printf("Uncaught exception: %s\n", e.what());
    return EXIT_FAILURE;
}