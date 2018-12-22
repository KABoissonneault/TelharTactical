#pragma once

#include <memory>

#include "math/vector2.h"

struct SDL_Texture;

namespace sdl {
    struct texture_delete {
        void operator()(SDL_Texture*) const noexcept;
    };

    class texture {
    public:
        texture() = default;
        texture(SDL_Texture* texture);

        auto get_dimensions() const noexcept -> math::vector2i { return {width, height}; }
        auto get_texture() const noexcept -> SDL_Texture const* { return ptr.get(); }
        auto get_texture() noexcept -> SDL_Texture* { return ptr.get(); }

    private:
        std::unique_ptr<SDL_Texture, texture_delete> ptr;
        int width = 0, height = 0;
    };
}