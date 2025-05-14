#pragma once

#include <Adafruit_INA219.h>

namespace view {
	class c_battery : public singleton_t< c_battery > {
	private:
		Adafruit_INA219 m_ina219{};

		bool					m_is_charging{};
		float					m_voltage{};
		constexpr static float	k_charging_threshold = 10.f;

	public:
		c_battery() : m_ina219(), m_voltage(0) {}

		void process() {
			if (!m_ina219.begin()) {
				DBG(msg::err, "battery::process: failed to find ina219\n");
				esp_restart();
			}

			DBG(msg::pos, "battery::process: ina219 initialized\n");
		}

		void handle() {
			const float bus_voltage = m_ina219.getBusVoltage_V(),
						shunt_voltage = m_ina219.getShuntVoltage_mV();

			m_voltage = bus_voltage + (shunt_voltage / 1000);

			// @todo
			/*if (g_ctx.m_power_save)
				m_ina219.powerSave(true);*/
		}

		bool is_charging() {
			const float m_cur_mA = m_ina219.getCurrent_mA();

			return m_is_charging = m_cur_mA > k_charging_threshold;
		}

		/*
		 * ONLY for 3.7v
		 * 4.2v => 100%
		 * 3.7v => ~20-50%
		 * 3.3v => 0%
		 */
		float percent(float cur_voltage) {
			constexpr static float	k_min = 3.3f,
									k_max = 4.2f;

			cur_voltage = constrain(cur_voltage, k_min, k_max);

			return (cur_voltage - k_min) / (k_max - k_min);
		}

		float voltage() const { return m_voltage; }
	};

	auto& g_battery = c_battery::instance();
}
