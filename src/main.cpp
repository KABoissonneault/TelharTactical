#include "sdl/resource.h"
#include "sdl/macro.h"
#include "command_args.h"

#include "game/map.h"

#include <SDL.h>
#include <SDL_image.h>

#include <gsl/gsl_util>
#include <cstdio>

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
    auto const window_size = args.window_size.value_or(math::vector2i{1280, 720});
    SDL_Window* window;
    SDL_Renderer* renderer;
    KT_SDL_ENSURE(SDL_CreateWindowAndRenderer(window_size.x, window_size.y, 0, &window, &renderer));
    return {sdl::unique_window(window), sdl::unique_renderer(renderer)};
}

bool is_quit_event(SDL_Event const& e) {
    return e.type == SDL_QUIT || e.type == SDL_KEYUP && e.key.keysym.sym == SDLK_ESCAPE;
}

void run_loop(command_args const& args) {
    // Init
    sdl_video_resources const video = initialize_sdl_video(args);
    auto const renderer = video.renderer.get();

    SDL_Texture* tileset = IMG_LoadTexture(renderer, "res/test_tileset.png");
    KT_SDL_FAILURE_IF(tileset == nullptr);
   
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
        SDL_RenderPresent(renderer);
    }
}

int main(int argc, char** argv) try {
    for(int i = 0; i < argc; ++i) {
        std::printf("Arg %d: %s\n", i, argv[i]);
    }

    command_args const args = parse_args({argv, argc});

    KT_SDL_ENSURE(SDL_Init(SDL_INIT_VIDEO));
    auto const sdl_destroy = gsl::finally(&SDL_Quit);

    KT_SDL_ENSURE(IMG_Init(IMG_INIT_PNG));
    auto const img_destroy = gsl::finally(&IMG_Quit);

    if(args.print_video_drivers) {
        print_video_drivers();
    }

    run_loop(args);

    return EXIT_SUCCESS;
} catch(std::exception const& e) {
    std::printf("Uncaught exception: %s\n", e.what());
    return EXIT_FAILURE;
}