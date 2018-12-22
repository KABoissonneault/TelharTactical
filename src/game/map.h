#pragma once

#include "tile.h"
#include "tileset.h"

struct SDL_Renderer;

namespace game {
    struct map_chunk {
        tile_chunk chunk;
        math::vector2i position;
    };
    
    class map {
    public:
        map() = default;
        template<typename MapChunkRange>
        map(tileset tileset, MapChunkRange&& map_chunks)
            : tileset(std::move(tileset))
            , chunks(std::begin(map_chunks), std::end(map_chunks)) {

        }
        void render(SDL_Renderer* renderer);

    private:        
        tileset tileset;
        std::vector<map_chunk> chunks;
    };
}