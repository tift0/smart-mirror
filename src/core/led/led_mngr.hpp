#pragma once

#include "core/cfg/cfg.hpp"

#include "sdk/math/datatypes/color.hpp"

namespace core {
	class c_led_mngr : public singleton_t< c_led_mngr > {
	private:
		enum e_pin { red = 0, green = 0, blue = 0, brightness = 0 }; // @todo

		math::clr_t m_clr{};
		int			m_brightness{};

	public:
		void setup_observers() {
			g_cfg_mngr.add_observer(
				"led_clr_r", [ & ](const std::string& key, const JsonVariant& value) {
					m_clr.r(value.as< float >());
				}
			);

			g_cfg_mngr.add_observer(
				"led_clr_g", [ & ](const std::string& key, const JsonVariant& value) {
					m_clr.g(value.as< float >());
				}
			);

			g_cfg_mngr.add_observer(
				"led_clr_b", [ & ](const std::string& key, const JsonVariant& value) {
					m_clr.b(value.as< float >());
				}
			);

			g_cfg_mngr.add_observer(
				"led_clr_a", [ & ](const std::string& key, const JsonVariant& value) {
					m_brightness = value.as< int >();
				}
			);
		}

		void process() {
			setup_observers();

			pinMode(red, OUTPUT);
			pinMode(green, OUTPUT);
			pinMode(blue, OUTPUT);
		}

		void handle() {
			// the analogWrite has a limit of acceptable values [0-1023] => we should clamp the values b4 writing them
			const int	vis_r = 1023 * m_clr.r(),
						vis_g = 1023 * m_clr.g(),
						vis_b = 1023 * m_clr.b();

			analogWrite(red, vis_r);
			analogWrite(green, vis_g);
			analogWrite(blue, vis_b);

			analogWrite(brightness, m_brightness);

			delay(10);
		}
	};

    auto& g_led_mngr = c_led_mngr::instance();
}
