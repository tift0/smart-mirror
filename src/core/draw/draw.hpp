#pragma once

#include <Adafruit_ST7735.h>
#include <Adafruit_GFX.h>

#include "sdk/singleton/singleton.hpp"
#include "sdk/math/vec2.hpp"
#include "sdk/msg/msg.hpp"

namespace core {
	enum e_align { none, center, center_y, right };

	// @reference: Adafruit_ST77xx.h
	enum e_clr : std::uint16_t {
		black = 0x0000, white = 0xFFFF, red = 0xF800,
		green = 0x07E0, blue = 0x001F, cyan = 0x07FF,
		magenta = 0xF81F, yellow = 0xFFE0, orange = 0xFC00
	};

	class c_draw : public singleton_t< c_draw > {
	private:
		enum e_pinout { e_cs = 13, e_dc = 2, e_rst = 4, e_mosi = 23, e_sclk = 18 };

		Adafruit_ST7735*	m_display{};
		bool				m_is_initialized{};

		template < typename _ty = std::uint16_t >
		math::vec2_t< _ty > measure_text(const String& text, const std::uint8_t font_scale = 1u) {
			if (!is_valid()) {
				DBG(msg::err, "measure_text: invalid display or renderer");
				return { 0, 0 };
			}

			if (text.isEmpty()) {
				DBG(msg::err, "measure_text: empty text\n");
				return { 0, 0 };
			}

			// yep
			constexpr int	k_base_width = 6u,
							k_base_height = 8u;

			return {
				static_cast< _ty >(text.length() * (k_base_width * font_scale)),
				static_cast< _ty >(k_base_height * font_scale)
			};
		}

		template < typename _ty_p = int, typename _ty_s = int >
		math::vec2_t< _ty_p > adjust_position(
			math::vec2_t< _ty_p > pos, math::vec2_t< _ty_s > size, const e_align flags
		) {
			_ty_p	new_x = pos.x(),
					new_y = pos.y();

			switch (flags) {
				case e_align::center:
					new_x = static_cast< _ty_p >(pos.x() - (size.x() / 2));
					break;
				case e_align::center_y:
					new_y = static_cast< _ty_p >(pos.y() - (size.y() / 2));
					break;
				case e_align::right:
					new_x = static_cast< _ty_p >(pos.x() - size.x());
					break;
				case e_align::none:
				default:
					break;
			}

			return { new_x, new_y };
		}

		template < typename _ty_p = int, typename _ty_s = int >
		void clear_text_area(math::vec2_t< _ty_p > pos, math::vec2_t< _ty_s > size) const {
			constexpr static int k_padding = 1;

			m_display->writeFillRect(
				pos.x() - k_padding,
				pos.y() - k_padding,
				size.x() + k_padding * 2,
				size.y() + k_padding * 2,
				e_clr::black
			);
			m_display->endWrite();
		}

	public:
		bool is_valid() const { return m_is_initialized && m_display != nullptr; }

		void process() {
			if (is_valid())
				return;

			m_display = new Adafruit_ST7735(
				e_pinout::e_cs, e_pinout::e_dc, e_pinout::e_mosi, e_pinout::e_sclk, e_pinout::e_rst
			);

			m_display->initR(INITR_BLACKTAB);
			m_display->setRotation(3u);
			m_display->fillScreen(e_clr::black);

			m_is_initialized = true;
		}

		// @todo
		template < typename _ty = int >
		void draw_text(
			math::vec2_t< _ty > pos, const String& str, const std::uint16_t clr = e_clr::white,
			const int font_scale = 1, const e_align flags = e_align::none
		) {
			if (!is_valid()
				|| str.isEmpty())
				return;

			const auto size = measure_text(str, font_scale);
			pos = adjust_position(pos, size, flags);

			static String prev_str{};
			if (prev_str != str) {
				clear_text_area(pos, size);
				prev_str = str;
			}

			m_display->setTextSize(font_scale);
			m_display->setTextColor(clr, e_clr::black);
			m_display->setTextWrap(true);
			m_display->setCursor(pos.x(), pos.y());

			m_display->print(str);
		}

		template < typename _ty = int >
		void draw_line(math::vec2_t< _ty > start, math::vec2_t< _ty > end, const e_clr clr) {
			if (!is_valid())
				return;

			m_display->drawLine(start.x(), start.y(), end.x(), end.y(), clr);
		}

		template < typename _ty = int >
		void draw_rect(math::vec2_t< _ty > pos, math::vec2_t< _ty > size, const e_clr clr) {
			if (!is_valid())
				return;

			m_display->fillRect(pos.x(), pos.y(), size.x(), size.y(), clr);
		}

		template < typename _ty = int >
		void draw_circle(math::vec2_t< _ty > center, _ty radius, const e_clr clr, const bool filled = true) {
			if (!is_valid())
				return;

			filled ?
				m_display->fillCircle(center.x(), center.y(), radius, clr)
				: m_display->drawCircle(center.x(), center.y(), radius, clr);
		}

		template < typename _ty = int >
		math::vec2_t< _ty > screen_size() {
			if (!is_valid())
				return { 0, 0 };

			return {
				m_display->width(),
				m_display->height()
			};
		}
	};

	auto& g_draw = c_draw::instance();
}
