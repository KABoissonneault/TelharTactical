#include "sdl/ttf.h"

#include <SDL_ttf.h>

namespace sdl {
	auto open_font(char const* filename, int point_size) -> unique_font {
		return unique_font(TTF_OpenFont(filename, point_size));
	}

	bool font::load(char const* filename, int point_size) {
		m_font = open_font(filename, point_size);
		return m_font != nullptr;
	}

	int font::get_height() const {
		return TTF_FontHeight(m_font.get());
	}

	void font::set_style(font_style style) {
		TTF_SetFontStyle(m_font.get(), static_cast<int>(style));
	}

	font_style font::get_style() const noexcept {
		return static_cast<font_style>(TTF_GetFontStyle(m_font.get()));
	}

	int font::get_ascent() const {
		return TTF_FontAscent(m_font.get());
	}

	int font::get_descent() const {
		return TTF_FontDescent(m_font.get());
	}
	
	int font::get_skip() const {
		return TTF_FontLineSkip(m_font.get());
	}
	
	bool font::is_fixed_width() const {
		return TTF_FontFaceIsFixedWidth(m_font.get());
	}

}