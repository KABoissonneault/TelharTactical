#include "serial/tiled.h"

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

        auto parse_integer(nlohmann::json const& json, std::string_view field) -> tl::expected<int, error> {
            auto const field_value = json.find(field);
        	if(field_value == json.end()) {
                return invalid_argument(fmt::format("Could not find field '{}'", field));
        	}
        	if(!field_value->is_number_integer()) {
                return invalid_argument(fmt::format("Field '{}' was not an integer", field));
            }
        	return static_cast<int>(*field_value);
        }

    	auto parse_integer_default(nlohmann::json const& json, std::string_view field, int default_value) -> int {
            auto const field_value = json.find(field);
        	if(field_value == json.end() || !field_value->is_number_integer()) {
                return default_value;
        	}
            return *field_value;
        }

        auto parse_float(nlohmann::json const& json, std::string_view field) -> tl::expected<double, error> {
            auto const field_value = json.find(field);
            if (field_value == json.end()) {
                return invalid_argument(fmt::format("Could not find field '{}'", field));
            }
            if (!field_value->is_number_float()) {
                if (field_value->is_number_integer()) {
                    return static_cast<double>(*field_value);
                }
                return invalid_argument(fmt::format("Field '{}' was not a float", field));
            }
            return static_cast<double>(*field_value);
        }

        auto parse_float_default(nlohmann::json const& json, std::string_view field, double default_value) -> double {
            auto const field_value = json.find(field);
            if (field_value == json.end() || !(field_value->is_number_float() || field_value->is_number_integer())) {
                return default_value;
            }
            return *field_value;
        }

        auto parse_boolean(nlohmann::json const& json, std::string_view field) -> tl::expected<bool, error> {
            auto const field_value = json.find(field);
            if (field_value == json.end()) {
                return invalid_argument(fmt::format("Could not find field '{}'", field));
            }
            if (!field_value->is_boolean()) {
                return invalid_argument(fmt::format("Field '{}' was not a boolean", field));
            }
            return static_cast<bool>(*field_value);
        }

        auto parse_boolean_default(nlohmann::json const& json, std::string_view field, bool default_value) -> bool {
            auto const field_value = json.find(field);
            if (field_value == json.end() || !field_value->is_boolean()) {
                return default_value;
            }
            return *field_value;
        }
    	
        auto parse_string(nlohmann::json const& json, std::string_view field) -> tl::expected<std::string, error> {
            auto const field_value = json.find(field);
            if (field_value == json.end()) {
                return invalid_argument(fmt::format("Could not find field '{}'", field));
            } 
            if (!field_value->is_string()) {
                return invalid_argument(fmt::format("Field '{}' was not a string", field));
            }
        	return static_cast<std::string>(*field_value);
        }

    	template<typename String>
        auto parse_string_default(nlohmann::json const& json, std::string_view field, String&& default_value) -> std::string {
            auto const field_value = json.find(field);
            if (field_value == json.end() || !field_value->is_string()) {
                return std::string(std::forward<String>(default_value));
            }
            return static_cast<std::string>(*field_value);
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

        auto parse_tile_layer_data(nlohmann::json const& tile_layer) -> tl::expected<game::layer::tile_data, error> {
            auto chunks_result = parse_range(tile_layer, "chunks", parse_tile_chunk);
            if (!chunks_result) {
                return tl::make_unexpected(chunks_result.error());
            }

            return game::layer::tile_data{ *std::move(chunks_result) };
        }

    	// parses a #RRGGBB or #AARRGGBB color string into a RGBA32 structure
        auto parse_color(std::string_view color_hex) -> tl::expected<game::rgba32_color, error> {
            if(!(color_hex.size() == 7 || color_hex.size() == 9) || color_hex[0] != '#') {
                return invalid_argument(fmt::format("String was not a valid color string: \"{}\"", color_hex));
            }

            game::rgba32_color color;
        	if (color_hex.size() == 7) { // #RRGGBB string
                int const parsed = std::sscanf(color_hex.data(), "#%2hhx%2hhx%2hhx", &color.r, &color.g, &color.b);
                if (parsed != 3) {
                    return invalid_argument(fmt::format("String was not a valid color string: \"{}\"", color_hex));
                }
                color.a = 0xFF;
            } else { // #AARRGGBB
                int const parsed = std::sscanf(color_hex.data(), "#%2hhx%2hhx%2hhx%2hhx", &color.a, &color.r, &color.g, &color.b);
                if (parsed != 4) {
                    return invalid_argument(fmt::format("String was not a valid color string: \"{}\"", color_hex));
                }
            }

            return color;
        }

    	// Default values derived from: https://doc.mapeditor.org/en/stable/reference/json-map-format/#text
        auto parse_object_text_data(nlohmann::json const& text_field) -> tl::expected<game::text_data, error> {
            game::text_data data;
            data.text = parse_string_default(text_field, "text", "");

            {
                std::string const color_hex = parse_string_default(text_field, "color", "#000000");
                auto const color_result = parse_color(color_hex);
                if (!color_result) {
                    return tl::make_unexpected(color_result.error());
                }
                data.color = *color_result;
            }
        	
            data.font = parse_string_default(text_field, "fontfamily", "sans-serif");
            data.point_size = parse_integer_default(text_field, "pixelsize", 16);

            {
                std::string const valign_string = parse_string_default(text_field, "valign", "top");
                if (valign_string == "center") {
                    data.valign = game::text_data::vertical_alignment::center;
                } else if (valign_string == "bottom") {
                    data.valign = game::text_data::vertical_alignment::bottom;
                } else {
                    data.valign = game::text_data::vertical_alignment::top;
                }
            }

            {
                std::string const halign_string = parse_string_default(text_field, "halign", "left");
                if (halign_string == "center") {
                    data.halign = game::text_data::horizontal_alignment::center;
                } else if (halign_string == "right") {
                    data.halign = game::text_data::horizontal_alignment::right;
                } else if (halign_string == "justify") {
                    data.halign = game::text_data::horizontal_alignment::justified;
                } else {
                    data.halign = game::text_data::horizontal_alignment::left;
                }                
            }
        	
            data.wrap = parse_boolean_default(text_field, "wrap", false);
            data.kerning = parse_boolean_default(text_field, "kerning", true);
            data.bold = parse_boolean_default(text_field, "bold", false);
            data.italic = parse_boolean_default(text_field, "italic", false);
            data.underline = parse_boolean_default(text_field, "underline", false);
            data.strikethrough = parse_boolean_default(text_field, "strikethrough", false);
           
            return data;
        }
    	
    	auto parse_object(nlohmann::json const& json) -> tl::expected<game::object, error> {
            game::object object;

            auto const id_result = parse_integer(json, "id");
            if (!id_result) {
                return tl::make_unexpected(id_result.error());
            }
            object.id = static_cast<game::object::identifier>(*id_result);
        
            object.name = parse_string_default(json, "name", "");
            object.type = parse_string_default(json, "type", "");

            object.dimensions.x = static_cast<int>(parse_float_default(json, "width", 0.0));
            object.dimensions.y = static_cast<int>(parse_float_default(json, "height", 0.0));
        	object.position.x = static_cast<int>(parse_float_default(json, "x", 0.0));
            object.position.y = static_cast<int>(parse_float_default(json, "y", 0.0));
            object.rotation = parse_float_default(json, "rotation", 0.0);

        	if(auto const point_field = json.find("point"); point_field != json.end() && *point_field == true) {
                object.kind_data = game::point_data();
            } else if (auto const text_field = json.find("text"); text_field != json.end() && text_field->is_structured()) {
                auto text_result = parse_object_text_data(*text_field);
                if (!text_result) {
                    return tl::make_unexpected(text_result.error());
                }
                object.kind_data = *std::move(text_result);
            // TODO: handle other kinds
            } else {
                object.kind_data = game::rectangle_data();
            }
            return object;
    	}

        auto parse_object_layer_data(nlohmann::json const& object_layer) -> tl::expected<game::layer::object_data, error> {
            auto objects_result = parse_range(object_layer, "objects", parse_object);
            if (!objects_result) {
                return tl::make_unexpected(objects_result.error());
            }

            return game::layer::object_data{ *std::move(objects_result) };
        }
    	
        auto parse_layer(nlohmann::json const& layer) -> tl::expected<game::layer, error> {
            if(!layer.is_object()) {
                return invalid_argument("Layer not a valid Layer object");
            }

            auto const type = layer.find("type");
            if(type == layer.end()) {
                return invalid_argument("Layer not a valid Layer object");
            }
        	
            auto const id = layer.find("id");
            if(id == layer.end() || !id->is_number_integer()) {
                return invalid_argument("Layer had invalid id");
            }

            game::layer ret;
            ret.id = *id;           

        	// Handle type-specific data
            if (*type == "tilelayer") {
                auto tile_data = parse_tile_layer_data(layer);
            	if(!tile_data) {
                    return tl::make_unexpected(tile_data.error());
            	}

                ret.data = *tile_data;
            } else if (*type == "objectgroup") {
                auto object_data = parse_object_layer_data(layer);
                if (!object_data) {
                    return tl::make_unexpected(object_data.error());
                }

                ret.data = *object_data;
            } else {
                return invalid_argument(fmt::format("Layer type '{}' invalid or not supported.", *type));
            }

            return ret;
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

			auto const firstgid = tileset.find("firstgid");
			if(firstgid == tileset.end() || !firstgid->is_number()) {
				return invalid_argument("Invalid 'firstgid' field in tileset");
			}
			
            return game::tileset{*source, *firstgid};
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