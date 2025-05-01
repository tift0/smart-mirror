#pragma once

#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>

#include "sdk/singleton/singleton.hpp"
#include "sdk/math/vec2.hpp"

namespace core {
	enum e_flags { none, center, center_y, right };

	class c_renderer : public singleton_t< c_renderer > {
	private:
		Adafruit_ST7735 m_display{ 5, 4, 2 };

		math::vec2_t< float > measure_text(const std::string& text) {
			std::uint16_t w{}, h{};

			m_display.getTextBounds(text.c_str(), 0, 0, nullptr, nullptr, &w, &h);

			return { static_cast< float >(w), static_cast< float >(h) };
		}

		math::vec2_t< std::int16_t > adjust_position(
			math::vec2_t< std::int16_t > pos, math::vec2_t< float > size, const e_flags flags
		) {
			std::int16_t	new_x = pos.x(),
							new_y = pos.y();

			if (flags & e_flags::center)
				new_x -= static_cast< std::int16_t >(size.x() / 2.f);
			if (flags & e_flags::center_y)
				new_y -= static_cast< std::int16_t >(size.y() * 0.5f);
			if (flags & e_flags::right)
				new_x -= static_cast< std::int16_t >(size.x());

			return { new_x, new_y };
		}

	public:
		void process() {
			m_display.initR(INITR_BLACKTAB);
			m_display.fillScreen(ST77XX_BLACK);
		}

		void draw_line(math::vec2_t< std::int16_t > start, math::vec2_t< std::int16_t > end) {
			m_display.drawLine(start.x(), start.y(), end.x(), end.y(), ST77XX_WHITE);
		}

		void draw_rect(math::vec2_t< std::int16_t > pos, math::vec2_t< std::int16_t > size) {
			m_display.drawRect(pos.x(), pos.y(), size.x(), size.y(), ST77XX_WHITE);
		}

		void draw_text(math::vec2_t< std::int16_t > pos, const std::string& str, const e_flags flags) {
			const auto size = measure_text(str);

			pos = adjust_position(pos, size, flags);

			m_display.setCursor(pos.x(), pos.y());
			m_display.setTextColor(ST77XX_WHITE);
			m_display.setTextWrap(true);
			m_display.print(str.c_str());
		}

		void draw_bitmap(
			math::vec2_t< std::int16_t > pos, const uint8_t* bitmap_data, const uint16_t width, const uint16_t height
		) {
			m_display.drawBitmap(pos.x(), pos.y(), bitmap_data, width, height, ST77XX_WHITE);
		}
	};

	auto& g_renderer = c_renderer::instance();
}
