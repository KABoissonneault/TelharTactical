#pragma once

#include "layer.h"

namespace game {
    struct tileset {
        std::string source;
    };

    struct map {
        std::vector<layer> layers;
        std::vector<tileset> tilesets;
    };
}