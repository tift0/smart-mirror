#include <Arduino.h>

#include "sdk/msg/msg.hpp"

#include "core/include.hpp"

#include "view/include.hpp"

Adafruit_SSD1306 display(128, 64, &Wire, -1);

void setup() {
	Serial.begin(9600);
	Serial.println();

	Wire.begin();
	Wire.setClock(100000);

#if 1
	DBG(msg::none, "i2c devices: ");
	for (byte addr = 1; addr < 127; addr++) {
		Wire.beginTransmission(addr);
		const byte err = Wire.endTransmission();
		if (err == 0) {
			DBG(msg::none, "found at addr 0x");
			if (addr < 16) DBG(msg::none, "0");

			Serial.println(addr, HEX);
		}
	}
#endif

	core::g_cfg_mngr.process();

	if (ESP.getFreeHeap() < 10240) {
		DBG(msg::err, "not enough free heap\n");
		esp_restart();
	}

	if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
		DBG(msg::err, "ssd1306 alloc failed\n");
		esp_restart();
	}

	display.clearDisplay();
	display.display();

	core::g_renderer.set_buffer(display);

	if (!core::g_renderer.is_valid()) {
		DBG(msg::err, "renderer init failed\n");
		esp_restart();
	}

	core::g_wifi.process();

	//view::g_led_mngr.process();

	//view::g_battery.process();

	view::g_display.process();

	Serial.println("[+] basic checks passed");
}

void loop() {
	static std::uint32_t    last_save_time = 0u;
	constexpr std::uint32_t save_interval = 60000u; // once per minute

	const std::uint32_t cur_time = millis();
	if (cur_time - last_save_time >= save_interval) {
		core::g_cfg_mngr.save_file();
		last_save_time = cur_time;
	}

	core::g_wifi.handle();

	core::g_notice_mngr.handle();

	//view::g_led_mngr.handle();

	//view::g_battery.handle();

	core::g_renderer.handle(
		[ & ] {
			view::g_display.handle();
		}
	);

	delay(50);
}
