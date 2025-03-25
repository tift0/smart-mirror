#include <Arduino.h>

#include "utilities/msg.hpp"
#include "backend/renderer/renderer.hpp"
#include "backend/wifi/wifi.hpp"

using namespace backend;

void setup( ) {
	Serial.begin( 9600 );

	c_wifi wifi( "espwfsm", "iforgotthepassword", 80 );

	wifi.process( );
}

void loop( ) {
	std::uint32_t now = millis( ),
					last_heap{};
	if ( now - last_heap >= 2000 ) {
		Serial.printf( "Free heap: %" PRIu32 "\n", ESP.getFreeHeap( ) );
		last_heap = now;
	}
}
