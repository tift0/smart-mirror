#include <Arduino.h>

#include "sdk/msg/msg.hpp"

#include "core/include.hpp"

#include "view/include.hpp"

void setup() {
	Serial.begin(9600);
	Serial.println();

	core::g_cfg_mngr.process();

	core::g_renderer.process();

	if (ESP.getFreeHeap() < 10240) {
		DBG(msg::err, "not enough free heap\n");
		esp_restart();
	}

	if (!core::g_renderer.is_valid()) {
		DBG(msg::err, "renderer init failed\n");
		esp_restart();
	}

	core::g_wifi.process();

	view::g_battery.process();

	view::g_display.process();

	DBG(msg::pos, "basic checks passed\n");
}

void loop() {
	core::g_cfg_mngr.handle();

	core::g_wifi.handle();

	core::g_notice_mngr.handle();

	view::g_battery.handle();

	view::g_display.handle();
}
