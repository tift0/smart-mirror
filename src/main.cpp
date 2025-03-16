#include <Arduino.h>

#include "utilities/msg.hpp"
#include "backend/renderer/renderer.hpp"
#include "backend/bluetooth/bluetooth.hpp"

using namespace backend;

U8G2_SSD1306_128X64_NONAME_F_HW_I2C display_buf( U8G2_R0, U8X8_PIN_NONE );

void setup( ) {
	Serial.begin( 9600 );

	display_buf.begin( );

	g_client.init( "espblesm" );

	Serial.println( ( uintptr_t )&display_buf, HEX );
}

// @todo: need an area for rendering only cuz we can't use call multiple times to clear/send the buffer
void loop( ) {
	/* @msg: maybe you could suggest a better way to approach this? */
	/* renderer area */
	g_renderer.clear_buffer( display_buf );

	g_renderer.draw_string( display_buf, math::vec2_t{ 10, 10 }, "meeeoeoeew" );

	g_renderer.send_buffer( display_buf );

	/* renderer area */
}
