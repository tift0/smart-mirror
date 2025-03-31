#include <Arduino.h>

#include "utilities/message/message.hpp"
#include "core/renderer/renderer.hpp"
#include "core/notify/notify.hpp"
#include "core/wifi/wifi.hpp"

void setup( ) {
	Serial.begin( 9600 );

	core::g_renderer.process( );

	core::g_wifi.process( );
}

void loop( ) {
	core::g_wifi.handle( );

	core::g_notice_mngr.handle( );
}
