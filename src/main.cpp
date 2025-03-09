#include <Arduino.h>
#include <WiFi.h>

#include "utilities/string.hpp"
#include "backend/renderer/renderer.hpp"

U8G2_SSD1306_128X64_NONAME_F_HW_I2C display_buf( U8G2_R0, U8X8_PIN_NONE );

String   ssids[ 256 ]{};
int      wifi_signal_strengths[ 256 ]{}, wifi_net_cnt{};
uint32_t last_wifi_scan{};

void wifi_scan( String* ssids, int* signal_strengths, int& net_count ) {
	net_count = WiFi.scanNetworks( );

	for ( int i = 0; i < net_count; i++ ) {
		ssids[ i ] = WiFi.SSID( i );
		signal_strengths[ i ] = WiFi.RSSI( i );
	}
}

void send_str( String str, int padding ) {
	backend::g_renderer.clear_buffer( display_buf );
	backend::g_renderer.draw_string( display_buf, math::vec2_t{ 0, 10 + padding }, str );
	backend::g_renderer.send_buffer( display_buf );
}

void setup( ) {
	Serial.begin( 9600 );
	display_buf.begin( );
	WiFi.mode( WIFI_MODE_STA );
	WiFi.disconnect( );
}

void loop( ) {
	unsigned long cur_millis = millis( );

	if ( cur_millis - last_wifi_scan >= 5000 ) {
		last_wifi_scan = cur_millis;
		wifi_scan( ssids, wifi_signal_strengths, wifi_net_cnt );
		Serial.print( "wifi_cnt: " );
		Serial.println( wifi_net_cnt );

		char buf[ 30 ]{};
		string::int_to_str( wifi_net_cnt, buf );
		send_str( string::cat_str( "wifi_cnt: ", buf ), 0 );
		send_str( "meeeoooowwe", 10 );

		delay( 1000 );
	}
}
