#include "sdl/resource.h"

#include <SDL_video.h>
#include <SDL_render.h>

namespace sdl {
    void window_delete::operator()(SDL_Window* p) const noexcept {
        if(p != nullptr) {
            SDL_DestroyWindow(p);
        }
    }
    
    void renderer_delete::operator()(SDL_Renderer* p) const noexcept {
        if(p != nullptr) {
            SDL_DestroyRenderer(p);
        }
    }
}