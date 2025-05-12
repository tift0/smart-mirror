#pragma once

#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>

#include "sdk/singleton/singleton.hpp"
#include "sdk/math/vec2.hpp"

namespace core {
	enum e_flags { none, center, center_y, right };

	class c_renderer : public singleton_t< c_renderer > {
	private:
		Adafruit_SSD1306* m_display{};
		bool              m_is_initialized{};

		template < typename _ty = std::uint16_t >
		math::vec2_t< _ty > measure_text(const String& text) {
			if (!is_valid() || !m_display) {
				DBG(msg::err, "measure_text: invalid display or renderer\n");
				return { 0, 0 };
			}

			if (text.isEmpty()) {
				DBG(msg::inf, "measure_text: empty text\n");
				return { 0, 0 };
			}

			auto len = text.length();
			DBG(msg::inf, "measure_text: text length: %d\n", len);

			std::uint16_t width{}, height{};
			try {
				m_display->getTextBounds(text, 0, 0, nullptr, nullptr, &width, &height);
			} catch (...) {
				DBG(msg::err, "measure_text: exception in getTextBounds\n");
				return { 0, 0 };
			}

			return { static_cast< _ty >(width), static_cast< _ty >(height) };
		}

		template < typename _ty_p = std::uint16_t, typename _ty_s = std::uint16_t >
		math::vec2_t< _ty_p > adjust_position(
			math::vec2_t< _ty_p > pos, math::vec2_t< _ty_s > size, const e_flags flags
		) {
			_ty_p	new_x = pos.x(),
				new_y = pos.y();

			if (flags & e_flags::center)
				new_x -= static_cast< _ty_p >(size.x() / 2.f);
			if (flags & e_flags::center_y)
				new_y -= static_cast< _ty_p >(size.y() * 0.5f);
			if (flags & e_flags::right)
				new_x -= static_cast< _ty_p >(size.x());

			return { new_x, new_y };
		}

	public:
		bool is_valid() const { return m_is_initialized && m_display != nullptr; }

		void process() {
			// ...
		}

		void clear_buffer() const {
			if (!is_valid()) {
				DBG(msg::err, "renderer not valid #1\n");
				return;
			}

			m_display->clearDisplay();
		}

		void send_buffer() const {
			if (!is_valid()) {
				DBG(msg::err, "renderer not valid #2\n");
				return;
			}

			m_display->display();
		}

		template < typename _fn >
		void handle(_fn&& fn) {
			clear_buffer();

			fn();

			send_buffer();
		}

		void set_buffer(Adafruit_SSD1306& display) {
			DBG(msg::inf, "set_buffer: starting...\n");
			m_display = &display;
			m_is_initialized = true;

			if (!m_display) {
				DBG(msg::err, "set_buffer: display pointer is null after assignment\n");
				return;
			}

			DBG(msg::inf, "set_buffer: configuring display...\n");
			m_display->setTextSize(1);
			m_display->setTextColor(SSD1306_WHITE);
			m_display->setTextWrap(false);
			DBG(msg::inf, "set_buffer: completed\n");
		}

		template < typename _ty = std::uint16_t >
		void draw_text(math::vec2_t< _ty > pos, const String& str, const e_flags flags = none) {
			if (!is_valid() || !m_display) {
				DBG(msg::err, "draw_text: invalid display or renderer\n");
				return;
			}

			DBG(msg::inf, "draw_text: measuring text...\n");
			const auto size = measure_text(str);

			DBG(msg::inf, "draw_text: adjusting position...\n");
			pos = adjust_position(pos, size, flags);

			DBG(msg::inf, "draw_text: setting display properties...\n");
			m_display->setTextSize(1);
			m_display->setTextColor(SSD1306_WHITE);
			m_display->setCursor(pos.x(), pos.y());

			DBG(msg::inf, "draw_text: printing text...\n");
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
