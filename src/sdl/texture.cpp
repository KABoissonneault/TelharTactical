#include "texture.h"

#include "sdl/macro.h"

#include <SDL_render.h>

namespace sdl {
    void texture_delete::operator()(SDL_Texture* p) const noexcept {
        if(p != nullptr) {
            SDL_DestroyTexture(p);
        }
    }

    texture::texture(SDL_Texture* texture)
        : ptr(texture) {
        KT_SDL_ENSURE(SDL_QueryTexture(ptr.get(), nullptr, nullptr, &width, &height));
    }
}