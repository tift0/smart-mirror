#include <Arduino.h>

#include "sdk/msg/msg.hpp"

#include "core/include.hpp"

#include "view/include.hpp"

void setup() {
	Serial.begin(9600);
	Serial.println();

	core::g_led_mngr.process();

	core::g_cfg_mngr.process();

	core::g_renderer.process();

	core::g_wifi.process();

	view::g_battery.process();
}

void loop() {
	static std::uint32_t last_save_time = 0u;
	constexpr std::uint32_t save_interval = 60000u; // once per minute

	const std::uint32_t cur_time = millis();
	if (cur_time - last_save_time >= save_interval) {
		core::g_cfg_mngr.save_file();
		last_save_time = cur_time;
	}

	core::g_led_mngr.handle();

	core::g_wifi.handle();

	core::g_notice_mngr.handle();

	view::g_battery.handle();
}
