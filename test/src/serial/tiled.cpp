#include <catch.hpp>

#include "serial/tiled.h"

#include <sstream>
#include <fstream>

TEST_CASE("Tiled Basic JSON", "[serial]") {
    std::stringstream ss;
    ss << "{";
    REQUIRE(!serial::load_tiled_json(ss));
    ss << "{ hey: \"hey\" }";
    REQUIRE(!serial::load_tiled_json(ss));
}

TEST_CASE("Tiled valid map", "[serial]") {
    std::fstream fs("res/test_infinite_map.json");
    REQUIRE(fs);
    
    auto const result = serial::load_tiled_json(fs);
    REQUIRE(result);
    
    game::map const& map = *result;
    REQUIRE(map.layers.size() == 1);
    
    game::layer const& tile_layer = map.layers[0];
    REQUIRE(tile_layer.id == game::layer::id_t{1});
    REQUIRE(tile_layer.get_type() == game::layer::type::tile);
    auto const& data = std::get<game::layer::tile_data>(tile_layer.data);
    REQUIRE(data.chunks.size() == 3);
    
    auto const it_found = std::find_if(data.chunks.begin(), data.chunks.end(), 
                 [] (game::tile_chunk const& chunk) { return chunk.position == math::vector2i{0, 0}; });
    REQUIRE(it_found != data.chunks.end());
    game::tile_chunk const& middle_chunk = *it_found;
    REQUIRE(middle_chunk.tiles.size() == game::tile_chunk::dimensions.x * game::tile_chunk::dimensions.y);
    REQUIRE(std::all_of(middle_chunk.tiles.begin(), middle_chunk.tiles.end(), 
                        [] (game::tile t) { return static_cast<int>(t.data) >= 1 && static_cast<int>(t.data) <= 4; }));
}

TEST_CASE("Tiled valid tileset", "[serial]") {
    std::fstream fs("res/test_tileset.json");
    REQUIRE(fs);

    auto const result = serial::get_tiled_tileset_image(fs);
    REQUIRE(result);

    auto const& image_file = *result;
    REQUIRE(image_file == "test_tileset.png");
}