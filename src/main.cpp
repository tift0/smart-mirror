#include <Arduino.h>

#include "utilities/msg.hpp"
#include "backend/renderer/renderer.hpp"
#include "backend/wifi/wifi.hpp"

using namespace backend;

c_wifi wifi( "espwfsm", "iforgotthepassword", 80 );

void setup( ) {
	Serial.begin( 9600 );

	wifi.process( );
}

void loop( ) {
	wifi.handle( );
}
