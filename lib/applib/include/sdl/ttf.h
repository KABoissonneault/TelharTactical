#pragma once

#include <SDL_ttf.h> // no forward declare for TTF_Font
#include <memory>

namespace sdl {
	struct font_delete {
		void operator()(TTF_Font* ptr) const noexcept { TTF_CloseFont(ptr); }
	};
	
	using unique_font = std::unique_ptr<TTF_Font, font_delete>;

	[[nodiscard]] auto open_font(char const* filename, int point_size) -> unique_font;

	enum class font_style {
		normal = TTF_STYLE_NORMAL,
		bold = TTF_STYLE_BOLD,
		italic = TTF_STYLE_ITALIC,
		underline = TTF_STYLE_UNDERLINE,
		strikethrough = TTF_STYLE_STRIKETHROUGH,
	};

	[[nodiscard]] inline font_style operator|(font_style lhs, font_style rhs) noexcept
	{
		return static_cast<font_style>(static_cast<int>(lhs) | static_cast<int>(rhs));
	}
	inline font_style& operator|=(font_style& lhs, font_style rhs) noexcept
	{
		return lhs = (lhs | rhs);
	}

	[[nodiscard]] inline font_style operator&(font_style lhs, font_style rhs) noexcept
	{
		return static_cast<font_style>(static_cast<int>(lhs) & static_cast<int>(rhs));
	}
	inline font_style& operator&=(font_style& lhs, font_style rhs) noexcept
	{
		return lhs = (lhs & rhs);
	}

	[[nodiscard]] inline bool has_flag(font_style lhs, font_style rhs) noexcept {
		return (lhs & rhs) == rhs;
	}
	
	// Class wrapping simple font operations
	class font {
		unique_font m_font;
	public:
		font() = default;
		font(unique_font font) noexcept
			: m_font(std::move(font))
		{
			
		}
		font& operator=(unique_font font) noexcept
		{
			m_font = std::move(font);
			return *this;
		}		

		[[nodiscard]] TTF_Font* get_font() const noexcept { return m_font.get(); }
		
		// Returns false if the font failed to load
		bool load(char const* filename, int point_size);
		void unload() noexcept { m_font.reset(); }
		// Returns true if it has a loaded font
		[[nodiscard]] bool is_loaded() const noexcept { return m_font != nullptr; }
		
		// Height in pixels of the font - usually equal to the point size
		// Expects: Loaded font
		[[nodiscard]] int get_height() const;
		// Max upward distance from baseline of the font
		// Expects: Loaded font
		[[nodiscard]] int get_ascent() const;
		// Max downward distance from baseline of the font
		// Expects: Loaded font
		[[nodiscard]] int get_descent() const;
		// Recommended pixel height of a rendered line of text of the current font
		// Expects: Loaded font
		[[nodiscard]] int get_skip() const;
		// Returns whether the font is fixed width (aka monospace)
		// Expects: Loaded font
		[[nodiscard]] bool is_fixed_width() const;
		
		void set_style(font_style style);
		[[nodiscard]] font_style get_style() const noexcept;
	};
}
