//
// created by tift0 on 09.03.2025.
//

#pragma once

#include <U8g2lib.h>

#include "../math/math.hpp"

namespace backend {
	class c_renderer {
	public:
		void clear_buffer( U8G2& buffer ) { buffer.clearBuffer( ); }

		void send_buffer( U8G2& buffer ) { buffer.sendBuffer( ); }

		void draw_string( U8G2& buffer, math::vec2_t pos, const String& str ) {
            buffer.setFont( u8g2_font_ncenB08_tr );
            buffer.drawStr( pos.x( ), pos.y( ), str.c_str( ) );
		}

		void draw_line( U8G2& buffer, math::vec2_t start, math::vec2_t end ) {
            buffer.drawLine( start.x( ), start.y( ), end.x( ), end.y( ) );
		}

		void draw_rect( U8G2& buffer, math::vec2_t pos, math::vec2_t size ) {
            buffer.drawBox( pos.x( ), pos.y( ), size.x( ), size.y( ) );
		}
	};

	extern c_renderer g_renderer{};
};
