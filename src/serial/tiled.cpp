#include "tiled.h"

#include <fmt/format.h>
#include <nlohmann/json.hpp>

#include <fstream>
#include <string>
#include <cctype>

#include <SDL_render.h>
#include <SDL_image.h>

using namespace std::string_literals;

namespace serial {
    namespace {
        template<typename StringT>
        auto invalid_argument(StringT&& str) {
            return tl::make_unexpected(error{std::make_error_code(std::errc::invalid_argument), std::forward<StringT>(str)});
        }

        // Utility function to collapse all the intermediary "errors" of a JSON array into a single structure 
        template<typename F>
        auto parse_range(nlohmann::json const& array, F f) 
            -> tl::expected<std::vector<typename decltype(f(std::declval<nlohmann::json>()))::value_type>, error> {
            using result_type = typename decltype(f(*array.begin()))::value_type;

            std::vector<result_type> output;
            for(auto const& element : array) {
                auto const parse_result = f(element);
                if(!parse_result) {
                    return tl::make_unexpected(parse_result.error());
                } else {
                    output.push_back(*std::move(parse_result));
                }
            }
            
            return output;
        }
        
        template<typename F>
        auto parse_range(nlohmann::json const& object, std::string_view field_name, F f)
            ->tl::expected<std::vector<typename decltype(f(std::declval<nlohmann::json>()))::value_type>, error> {
            auto const field = object.find(field_name);
            if(field == object.end() || !field->is_array()) {
                return invalid_argument(fmt::format("Expected '{}' array field", field_name));
            }

            return parse_range(*field, f);
        }

        auto parse_tile_chunk(nlohmann::json const& chunk) -> tl::expected<game::tile_chunk, error> {
            auto const width = chunk.find("width");
            auto const height = chunk.find("height");
            if(width == chunk.end() || *width != game::tile_chunk::dimensions.x 
               || height == chunk.end() || *height != game::tile_chunk::dimensions.y) {
                return invalid_argument(
                    fmt::format("Chunk had invalid dimensions, expected ({},{})", 
                                game::tile_chunk::dimensions.x, game::tile_chunk::dimensions.y
                    )
                );
            }

            auto const x = chunk.find("x");
            auto const y = chunk.find("y");
            if(x == chunk.end() || y == chunk.end() || !x->is_number_integer() || !y->is_number_integer()) {
                return invalid_argument("Chunk had invalid 'x' and 'y' fields");
            }

            auto const tiles = chunk.find("data");
            if(tiles == chunk.end() || !tiles->is_array()) {
                return invalid_argument("Chunk had invalid 'data' field");
            }

            game::tile_chunk result{*x, *y};
            result.tiles.reserve(tiles->size());
            for(auto const& tile : *tiles) {
                if(!tile.is_number_unsigned()) {
                    return invalid_argument("A tile was not a positive integer");
                }
                result.tiles.push_back(game::tile{tile});
            }

            return result;
        }

        auto parse_layer(nlohmann::json const& layer) -> tl::expected<game::layer, error> {
            if(!layer.is_object()) {
                return invalid_argument("Layer not a valid Layer object");
            }

            auto const type = layer.find("type");
            if(type == layer.end() || *type != "tilelayer") {
                return invalid_argument("Layer not a tile layer");
            }

            auto const id = layer.find("id");
            if(id == layer.end() || !id->is_number_integer()) {
                return invalid_argument("Layer had invalid id");
            }
           
            // Might handle more layers eventually
            assert(*type == "tilelayer");
            
            auto const chunks_result = parse_range(layer, "chunks", parse_tile_chunk);
            if(!chunks_result) {
                return tl::make_unexpected(chunks_result.error());
            }

            return game::layer{*id, game::layer::tile_data{*chunks_result}};
        }

        auto sanitize_map(nlohmann::json const& json) -> tl::expected<nlohmann::json, error> {
            auto const version = json.find("tiledversion");
            if(version == json.end()) {
                return invalid_argument("Not a Tiled map");
            }
            auto const version_string = version->get<std::string>();
            if(version_string.size() < 5 || version_string[1] != '.' || version_string[3] != '.') {
                return invalid_argument(fmt::format("Tiled version invalid, was {}", version_string));
            }
            char const major = version_string[0], minor = version_string[2];
            if(!std::isdigit(major) || !std::isdigit(minor)) {
                return invalid_argument(fmt::format("Invalid Tiled invalid, expected 'x.y.z', was {}", version_string));
            }

            if(major != '1' || minor < '2') {
                return invalid_argument(fmt::format("Invalid Tiled version, expected at least 1.2, was {}", version_string));
            }

            if(auto const infinite = json.find("infinite"); 
               infinite == json.end() || *infinite != true) {
                return invalid_argument("Not an infinite map");
            }

            if(auto const tilewidth = json.find("tilewidth"), tileheight = json.find("tileheight");
                 tilewidth == json.end() || *tilewidth != game::tile::dimensions.x 
               || tileheight == json.end() || *tileheight != game::tile::dimensions.y) {
                return invalid_argument(
                    fmt::format("Invalid 'tilewidth' or 'tileheight': expected ({}, {})", 
                                game::tile::dimensions.x, 
                                game::tile::dimensions.y)
                );
            }

            if(auto const renderorder = json.find("renderorder");
               renderorder == json.end() || *renderorder != "right-down") {
                return invalid_argument("Expected 'right-down' in the 'renderorder' field");
            }

            return json;
        }

        auto parse_tileset(nlohmann::json const& tileset) -> tl::expected<game::tileset, error> {
            auto const source = tileset.find("source");
            if(source == tileset.end() || !source->is_string()) {
                return invalid_argument("Invalid 'source' field in tileset");
            }

            return game::tileset{*source};
        }

        auto parse_map(nlohmann::json const& json) -> tl::expected<game::map, error> {
            return sanitize_map(json).and_then([] (nlohmann::json const& map) -> tl::expected<game::map, error> {
                auto layers_result = parse_range(map, "layers", parse_layer);
                if(!layers_result) {
                    return tl::make_unexpected(layers_result.error());
                }

                auto tilesets_result = parse_range(map, "tilesets", parse_tileset);
                if(!tilesets_result) {
                    return tl::make_unexpected(tilesets_result.error());
                }

               return game::map{*std::move(layers_result), *std::move(tilesets_result)};
            });
        }
    }

    auto load_tiled_json(std::istream& map_data) -> tl::expected<game::map, error> {
        auto const json = nlohmann::json::parse(map_data, nullptr, false);
        if(json.is_discarded()) {
            return invalid_argument("Input stream was not a valid JSON");
        }

        return parse_map(json).map_error([] (error e) -> error {
            return {e.code, "Map parse error: " + e.description};
        });
    }

    namespace {

    }

    auto get_tiled_tileset_image(std::istream& tileset_data) -> tl::expected<std::string, error> {
        auto const json = nlohmann::json::parse(tileset_data, nullptr, false);
        if(json.is_discarded()) {
            return invalid_argument("Input stream was not a valid JSON");
        }

        auto const width = json.find("tilewidth");
        if(width == json.end() || *width != game::tile::dimensions.x) {
            return invalid_argument(fmt::format("Tileset had invalid tile width: expected {}", game::tile::dimensions.x));
        }

        auto const height = json.find("tileheight");
        if(height == json.end() || *height != game::tile::dimensions.y) {
            return invalid_argument(fmt::format("Tileset had invalid tile height: expected {}", game::tile::dimensions.y));
        }

        auto const image = json.find("image");
        if(image == json.end() || !image->is_string()) {
            return invalid_argument("Invalid 'image' field");
        }

        return image->get<std::string>();
    }
}