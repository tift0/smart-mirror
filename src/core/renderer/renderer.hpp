#pragma once

#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>

#include "utilities/math/math.hpp"

// @todo: https://github.com/adafruit/Adafruit-ST7735-Library
namespace core {
	class c_renderer {
	private:
		Adafruit_ST7735 m_display;

	public:
		explicit c_renderer( const std::int8_t cs_pin, const std::int8_t volt_pin, const std::int8_t rst_pin )
			: m_display( cs_pin, volt_pin, rst_pin ) {
		}

		void process( ) {
			m_display.initR( INITR_BLACKTAB );
			m_display.fillScreen( ST77XX_BLACK );
		}

		void draw_line( math::vec2_t start, math::vec2_t end ) {
			m_display.drawLine( start.x( ), start.y( ), end.x( ), end.y( ), ST77XX_WHITE );
		}

		void draw_rect( math::vec2_t pos, math::vec2_t size ) {
			m_display.drawRect( pos.x(), pos.y(), size.x( ), size.y( ), ST77XX_WHITE );
		}

		void draw_text( math::vec2_t pos, const std::string& str ) {
			m_display.setCursor( pos.x( ), pos.y( ) );
			m_display.setTextColor( ST77XX_WHITE );
			m_display.setTextWrap( true );
			m_display.print( str.c_str( ) );
		}
	};

	c_renderer g_renderer( 4, 5, 16 );
};
