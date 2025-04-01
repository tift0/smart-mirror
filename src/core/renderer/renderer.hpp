#pragma once

#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>

#include "utilities/math/math.hpp"

/*
 * @todo:
 *		> https://github.com/adafruit/Adafruit-ST7735-Library
 *		> align flag
 */
namespace core {
	enum e_align { none, left, right, center };

	class c_renderer {
	private:
		Adafruit_ST7735 m_display;

	public:
		explicit c_renderer( const std::int8_t cs_pin, const std::int8_t data_cmd_pin, const std::int8_t rst_pin )
			: m_display( cs_pin, data_cmd_pin, rst_pin ) {
		}

		void process( ) {
			m_display.initR( INITR_BLACKTAB );
			m_display.fillScreen( ST77XX_BLACK );
		}

		void draw_line( math::vec2_t< std::int16_t > start, math::vec2_t< std::int16_t > end ) {
			m_display.drawLine( start.x( ), start.y( ), end.x( ), end.y( ), ST77XX_WHITE );
		}

		void draw_rect( math::vec2_t< std::int16_t > pos, math::vec2_t< std::int16_t > size ) {
			m_display.drawRect( pos.x( ), pos.y( ), size.x( ), size.y( ), ST77XX_WHITE );
		}

		void draw_text( math::vec2_t< std::int16_t > pos, const std::string& str, const e_align align ) {
			m_display.setCursor( pos.x( ), pos.y( ) );
			m_display.setTextColor( ST77XX_WHITE );
			m_display.setTextWrap( true );
			m_display.print( str.c_str( ) );
		}
	};

	c_renderer g_renderer( 5, 4, 2 );
};
