#pragma once

#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>

#include "sdk/singleton/singleton.hpp"
#include "sdk/math/vec2.hpp"
#include "sdk/msg/msg.hpp"

namespace core {
	enum e_align { none, center, center_y, right };

	class c_renderer : public singleton_t< c_renderer > {
	private:
		Adafruit_SSD1306* m_display{};
		bool              m_is_initialized{};

		void clear_buffer() const {
			if (!is_valid() || !m_display) {
				DBG(msg::err, "clear_buffer: invalid display or renderer\n");
				return;
			}

			m_display->clearDisplay();
		}

		void send_buffer() const {
			if (!is_valid() || !m_display) {
				DBG(msg::err, "send_buffer: invalid display or renderer\n");
				return;
			}

			m_display->display();
		}

		template < typename _ty = std::uint16_t >
		math::vec2_t< _ty > measure_text(const String& text, const std::uint8_t font_size = 1) {
			if (!is_valid() || !m_display) {
				Serial.println("measure_text: invalid display or renderer");
				return { 0, 0 };
			}

			if (text.isEmpty()) {
				Serial.println("measure_text: empty text");
				return { 0, 0 };
			}

			constexpr int k_base_width = 6;
			constexpr int k_base_height = 8;

			int width = text.length() * (k_base_width * font_size),
				height = k_base_height * font_size;

			return { static_cast< _ty >(width), static_cast< _ty >(height) };
		}

		template < typename _ty_p = std::uint16_t, typename _ty_s = std::uint16_t >
		math::vec2_t< _ty_p > adjust_position(
			math::vec2_t< _ty_p > pos, math::vec2_t< _ty_s > size, const e_align flags
		) {
			_ty_p	new_x = pos.x(),
					new_y = pos.y();

			switch (flags) {
				case e_align::center:
					new_x = static_cast<_ty_p>(pos.x() - (size.x() / 2));
					break;
				case e_align::center_y:
					new_y = static_cast<_ty_p>(pos.y() - (size.y() / 2));
					break;
				case e_align::right:
					new_x = static_cast<_ty_p>(pos.x() - size.x());
					break;
				case e_align::none:
				default:
					break;
			}

			return { new_x, new_y };
		}

	public:
		bool is_valid() const { return m_is_initialized && m_display != nullptr; }

		void process(Adafruit_SSD1306& display) {
			DBG(msg::inf, "renderer::process: starting...\n");

			m_display = &display;
			m_is_initialized = true;

			if (!m_display) {
				DBG(msg::err, "renderer::process: display pointer is null after assignment\n");
				return;
			}

			DBG(msg::inf, "renderer::process: configuring display...\n");
			try {
				m_display->setTextSize(1);
				m_display->setTextColor(SSD1306_WHITE);
				m_display->setTextWrap(false);
			} catch (...) {
				DBG(msg::err, "renderer::process: failed to configure display\n");
				return;
			}

			DBG(msg::inf, "renderer::process: completed\n");
		}

		template < typename _fn >
		void handle(_fn&& fn) {
			clear_buffer();

			std::forward< _fn >(fn)();

			send_buffer();
		}

		template < typename _ty = std::uint16_t >
		void draw_text(math::vec2_t< _ty > pos, const String& str, const std::uint8_t font_size = 1, const e_align flags = none) {
			if (!is_valid()
				|| !m_display)
				return;

			const auto size = measure_text(str, font_size);

			pos = adjust_position(pos, size, flags);

			m_display->setTextSize(1);
			m_display->setTextColor(SSD1306_WHITE);
			m_display->setCursor(pos.x(), pos.y());

			m_display->print(str.c_str());
		}

		template < typename _ty = std::uint16_t >
		void draw_line(math::vec2_t< _ty > start, math::vec2_t< _ty > end) {
			if (!is_valid()
				|| !m_display)
				return;

			m_display->drawLine(start.x(), start.y(), end.x(), end.y(), SSD1306_WHITE);
		}

		template < typename _ty = std::uint16_t >
		void draw_rect(math::vec2_t< _ty > pos, math::vec2_t< _ty > size) {
			if (!is_valid()
				|| !m_display)
				return;

			m_display->fillRect(pos.x(), pos.y(), size.x(), size.y(), SSD1306_WHITE);
		}

		template < typename _ty = std::uint16_t >
		math::vec2_t< _ty > screen_size() {
			if (!is_valid()
				|| !m_display)
				return { 0, 0 };

			auto	width = m_display->width(),
					height = m_display->height();

			return { width, height };
		}
	};

	auto& g_renderer = c_renderer::instance();
}
