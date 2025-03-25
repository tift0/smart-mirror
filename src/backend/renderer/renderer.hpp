// LIBRARY LCDWIKI OUTDATED
// @todo: https://github.com/adafruit/Adafruit-ST7735-Library

/*
#pragma once

#include <LCDWIKI_SPI.h>

#include "../math/math.hpp"

namespace backend {
	class c_renderer {
		LCDWIKI_SPI m_display;

	public:
		// @note: difference between ST7735S / ST7735S128 ?
		c_renderer( std::int8_t chip_sel, std::int8_t cmd_data, std::int8_t rst )
			: m_display( ST7735S, chip_sel, cmd_data, rst, -1 ) {
			m_display.Init_LCD( );
			m_display.Fill_Screen( 0x0000 );
		}

		void draw_str( const std::string& text, math::vec2_t pos, const std::uint8_t text_size ) {
			m_display.Set_Text_Back_colour( 0x0000 );
			m_display.Set_Text_colour( 0xffff );
			m_display.Set_Text_Size( text_size );
			m_display.Print_String( text.c_str( ), pos.x( ), pos.y( ) );
		}

		// @todo: + std::uint8_t round
		void draw_rect( math::vec2_t pos, math::vec2_t size ) {
			m_display.Draw_Rectangle( pos.x( ), pos.y( ), size.x( ), size.y( ) );
		}

		void draw_bitmap( math::vec2_t pos, math::vec2_t size, const std::uint16_t* data, std::uint16_t scale ) {
			m_display.Draw_Bit_Map( pos.x( ), pos.y( ), size.x( ), size.y( ), data, scale );
		}
	};

	extern c_renderer g_renderer;
};
*/
