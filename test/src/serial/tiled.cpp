#include <catch.hpp>

#include <serial/tiled.h>
#include "serial/test_tiled_map.h"
#include "serial/test_tileset.h"

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
	std::stringstream ss;
	ss << test_tiled_map;

    auto const result = serial::load_tiled_json(ss);
    REQUIRE(result);
    
    game::map const& map = *result;
    REQUIRE(map.layers.size() == 1);
    
	// Layer
    game::layer const& tile_layer = map.layers[0];
    REQUIRE(tile_layer.id == game::layer::id_t{1});
    REQUIRE(tile_layer.get_type() == game::layer::type::tile);
    auto const& data = std::get<game::layer::tile_data>(tile_layer.data);
    REQUIRE(data.chunks.size() == 10);
    auto const it_found = std::find_if(data.chunks.begin(), data.chunks.end(), 
                 [] (game::tile_chunk const& chunk) { return chunk.position == math::vector2i{0, 0}; });
    REQUIRE(it_found != data.chunks.end());
    game::tile_chunk const& middle_chunk = *it_found;
    REQUIRE(middle_chunk.tiles.size() == game::tile_chunk::dimensions.x * game::tile_chunk::dimensions.y);
    REQUIRE(std::all_of(middle_chunk.tiles.begin(), middle_chunk.tiles.end(), 
                        [] (game::tile t) { return static_cast<int>(t.data) >= 1 && static_cast<int>(t.data) <= 8; }));

	// Tilesets
	REQUIRE(map.tilesets.size() == 2);
	auto const it_test_tileset = std::find_if(map.tilesets.begin(), map.tilesets.end(), [] (game::tileset const& tileset) {
		return tileset.source == "test_tileset.json";
	});
	REQUIRE(it_test_tileset != map.tilesets.end());
	REQUIRE(it_test_tileset->source == "test_tileset.json");
	REQUIRE(it_test_tileset->starting_id == game::tile::id(1));
}

TEST_CASE("Tiled valid tileset", "[serial]") {
	std::stringstream ss;
	ss << test_tileset;
    REQUIRE(ss);

    auto const result = serial::get_tiled_tileset_image(ss);
    REQUIRE(result);

    auto const& image_file = *result;
    REQUIRE(image_file == "test_tileset.png");
}