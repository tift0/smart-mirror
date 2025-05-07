#pragma once

#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>

#include "sdk/singleton/singleton.hpp"
#include "sdk/math/vec2.hpp"

namespace core {
	enum e_flags { none, center, center_y, right };

	class c_renderer : public singleton_t< c_renderer > {
	private:
		enum e_pin { e_cs = 5, e_dc = 4, e_rst = 2 };

		Adafruit_ST7735 m_display{ e_cs, e_dc, e_rst };

		template < typename _ty = std::uint16_t >
		math::vec2_t< _ty > measure_text(const std::string& text) {
			std::uint16_t width{}, height{};

			m_display.getTextBounds(text.c_str(), 0, 0, nullptr, nullptr, &width, &height);

			return { static_cast< _ty >(width), static_cast< _ty >(height) };
		}

		template < typename _ty = std::uint16_t >
		math::vec2_t< _ty > adjust_position(
			math::vec2_t< _ty > pos, math::vec2_t< _ty > size, const e_flags flags
		) {
			_ty	new_x = pos.x(),
				new_y = pos.y();

			if (flags & e_flags::center)
				new_x -= static_cast< _ty >(size.x() / 2.f);
			if (flags & e_flags::center_y)
				new_y -= static_cast< _ty >(size.y() * 0.5f);
			if (flags & e_flags::right)
				new_x -= static_cast< _ty >(size.x());

			return { new_x, new_y };
		}

	public:
		void process() {
			m_display.initR(INITR_BLACKTAB);
			m_display.fillScreen(ST77XX_BLACK);
		}

		// p100
		bool is_valid() const { return m_display.width() > 0 && m_display.height() > 0; }

		template < typename _ty = std::uint16_t >
		void draw_line(math::vec2_t< _ty > start, math::vec2_t< _ty > end) {
			m_display.drawLine(start.x(), start.y(), end.x(), end.y(), ST77XX_WHITE);
		}

		template < typename _ty = std::uint16_t >
		void draw_rect(math::vec2_t< _ty > pos, math::vec2_t< _ty > size) {
			m_display.drawRect(pos.x(), pos.y(), size.x(), size.y(), ST77XX_WHITE);
		}

		template < typename _ty = std::uint16_t >
		void draw_text(math::vec2_t< _ty > pos, const std::string& str, const e_flags flags = e_flags::none) {
			const auto size = measure_text(str);

			pos = adjust_position(pos, size, flags);

			m_display.setCursor(pos.x(), pos.y());
			m_display.setTextColor(ST77XX_WHITE);
			m_display.setTextWrap(true);
			m_display.print(str.c_str());
		}

		template < typename _ty = std::uint16_t >
		void draw_bitmap(
			math::vec2_t< _ty > pos, const uint8_t* bitmap_data, const uint16_t width, const uint16_t height
		) {
			m_display.drawBitmap(pos.x(), pos.y(), bitmap_data, width, height, ST77XX_WHITE);
		}

		template < typename _ty = std::uint16_t >
		math::vec2_t< _ty > screen_size() {
			const auto	width = m_display.width(),
					height = m_display.height();

			return { width, height };
		}
	};

	auto& g_renderer = c_renderer::instance();
}
