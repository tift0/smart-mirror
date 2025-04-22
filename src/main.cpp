#include <Arduino.h>

#include "sdk/msg/msg.hpp"
#include "core/renderer/renderer.hpp"
#include "core/notify/notify.hpp"
#include "core/wifi/wifi.hpp"
#include "core/cfg/cfg.hpp"

void setup() {
	Serial.begin(9600);
	Serial.println();

	core::g_cfg_mngr.process();

	//core::g_renderer.process( );

	core::g_wifi.process();
}

void loop() {
	static std::uint32_t last_save_time = 0u;
	constexpr std::uint32_t save_interval = 60000u; // once per minute

	const std::uint32_t cur_time = millis();
	if (cur_time - last_save_time >= save_interval) {
		core::g_cfg_mngr.save_file();
		last_save_time = cur_time;
	}

	core::g_wifi.handle();

	core::g_notice_mngr.handle();
}
