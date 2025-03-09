//
// created by tift0 on 09.03.2025.
//

#include <U8g2lib.h>
#include "../math.hpp"

namespace backend {
    class c_renderer {
    public:
      	void clear_buffer( U8G2& buf ) {
      		buf.clearBuffer( );
      	}

    	void send_buffer( U8G2& buf ) {
      		buf.sendBuffer( );
      	}

		void draw_string( U8G2& buf, math::vec2_t pos, String str ) {
        	buf.setFont( u8g2_font_ncenB08_tr );
            buf.drawStr( pos.x( ), pos.y( ), str.c_str( ) );
		}
    };
    extern c_renderer g_renderer{};
};
