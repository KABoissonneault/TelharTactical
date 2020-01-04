#pragma once

#include "math/vector2.h"
#include "game/tile.h"

#include <string>
#include <variant>
#include <vector>

namespace game {
	// 32 bits, "RGBA" color format. Each color or alpha component is 8 bits
	struct rgba32_color {
		// red component of a color, out of 255
		std::uint8_t r;
		// green component of a color, out of 255
		std::uint8_t g;
		// blue component of a color, out of 255
		std::uint8_t b;
		// alpha component of a color, out of 255
		std::uint8_t a;
	};

	struct rectangle_data { };
	struct ellipse_data { };
	struct point_data { };
	struct polygon_data	{
		std::vector<math::vector2i> points;
	};
	struct polyline_data {
		std::vector<math::vector2i> points;
	};
	struct text_data {
		enum class vertical_alignment {
			top,
			center,
			bottom,
		};

		enum class horizontal_alignment {
			left,
			center,
			right,
			justified,
		};
		
		std::string text;
		rgba32_color color;
		std::string font;
		int point_size;
		vertical_alignment valign;
		horizontal_alignment halign;
		bool wrap;
		bool kerning;
		bool bold;
		bool italic;
		bool underline;
		bool strikethrough;
	};
	struct sprite_data {
		tile::id gid;
	};
	
	struct object {
		enum class identifier {};
		enum class kind { rectangle, point, ellipse, polygon, polyline, text, sprite };

		// Identity
		identifier id;
		std::string name;
		std::string type;
		
		// Physic
		double rotation;
		math::vector2i dimensions;
		math::vector2i position;

		std::variant<rectangle_data, point_data, ellipse_data, polygon_data, polyline_data, text_data, sprite_data> kind_data;

		[[nodiscard]] kind get_kind() const noexcept
		{
			return static_cast<kind>(kind_data.index());
		}
	};
}
