#include "map.h"

#include "sdl/macro.h"

#include <SDL_render.h>

namespace game {
    void map::render(SDL_Renderer* renderer) {
        constexpr int itile_pixel_length = tile_pixel_length;
        auto const chunk_pixel_length =tile_pixel_length  * chunk_length;
        for(auto const& map_chunk : chunks) {
            math::vector2i const chunk_pixel_position = map_chunk.position * chunk_pixel_length;
            auto const& tiles = map_chunk.chunk.tiles;
            for(int j = 0; j < tiles.size(); ++j) {
                auto const& row = tiles[j];
                for(int i = 0; i < row.size(); ++i) {
                    tile const& tile = row[i];
                    if(tile.index >= 0) {
                        auto const tileset_row_length = static_cast<int>(get_row_length(tileset));
                        auto const tile_rect_pos = math::vector2i{
                            tile.index % tileset_row_length,
                            tile.index / tileset_row_length
                        };
                        SDL_Rect const tile_rect{
                            tile_rect_pos.x * itile_pixel_length,
                            tile_rect_pos.y * itile_pixel_length,
                            itile_pixel_length,
                            itile_pixel_length
                        };
                        SDL_Rect const render_rect{
                            chunk_pixel_position.x + i*itile_pixel_length,
                            chunk_pixel_position.y + j*itile_pixel_length,
                            itile_pixel_length,
                            itile_pixel_length
                        };
                        KT_SDL_ENSURE(SDL_RenderCopy(renderer, tileset.texture.get_texture(), &tile_rect, &render_rect));
                    }
                }
            }
        }
        
    }
}