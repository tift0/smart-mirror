#pragma once

#include <vector>
#include <unordered_map>

#include <ArduinoJson.h>
#include <LittleFS.h>

#include "sdk/singleton/singleton.hpp"

namespace core {
	class c_cfg_manager : public singleton_t< c_cfg_manager > {
		using cfg_key = std::string;
		using json_value = JsonVariant;
		using change_callback = std::function< void(const cfg_key&, const json_value&) >;

	private:
		constexpr static auto	k_def_file = "/def_cfg.json",
								k_user_file = "/user_cfg.json";

		JsonDocument m_def_cfg{}, m_user_cfg{};

		bool m_is_edited{};

		std::unordered_map< cfg_key, std::vector< change_callback > > m_observers{};

	public:
		void process() {
			if (!LITTLEFS.begin(false)) {
				DBG(msg::warn, "cfg::process: littlefs mount failed, formatting...\n");
				if (LITTLEFS.format()) {
					DBG(msg::pos, "cfg::process: format successful, retrying mount...\n");
					if (!LITTLEFS.begin(false)) {
						DBG(msg::warn, "cfg::process: mount still failed after format\n");
						esp_restart();
					}
				} else {
					DBG(msg::neg, "cfg::process: format failed\n");
					esp_restart();
				}
			} else
				DBG(msg::pos, "cfg::process: littlefs mounted successfully\n");

			load_def_file();

			if (!load_user_file())
				DBG(msg::inf, "cfg::process: user config not found, using defaults\n");

			apply_def_file();
		}

		void handle() {
			static std::uint32_t    last_update{};
			constexpr static auto	k_update_delay = 30000u;

			const auto cur_time = millis();
			if (cur_time - last_update >= k_update_delay) {
				save_file();
				last_update = cur_time;
			}
		}

		bool set(const cfg_key& key, const json_value& value) {
			m_user_cfg[ key ] = value;
			m_is_edited = true;

			notify_observers(key, value);

			return true;
		}

		[[nodiscard]] json_value get(const cfg_key& key) {
			json_value result{};

			if (!m_user_cfg[ key ].isNull())
				result = m_user_cfg[ key ];
			else if (!m_def_cfg[ key ].isNull())
				result = m_def_cfg[ key ];

			return result;
		}

		void add_observer(const cfg_key& key, change_callback callback) {
			m_observers[ key ].push_back(std::move(callback));
		}

		bool save_file() {
			DBG(msg::inf, "cfg::save_file: save_file: m_is_edited = ");
			Serial.println(m_is_edited ? "true" : "false");

			if (!m_is_edited) {
				DBG(msg::inf, "cfg::save_file: save_file: no changes to save\n");
				return true;
			}

			auto file = LITTLEFS.open(k_user_file, FILE_WRITE);
			if (!file) {
				DBG(msg::err, "cfg::save_file: can't open user config\n");
				return false;
			}

			JsonDocument json{};
			for (JsonPair kv : m_user_cfg.as< JsonObject >())
				json[ kv.key() ] = kv.value();

			if (serializeJson(json, file) == 0) {
				DBG(msg::err, "cfg::save_file: failed to write json to file\n");
				file.close();
				return false;
			}

			file.close();
			m_is_edited = false;

			DBG(msg::pos, "cfg::save_file: config saved\n");

			return true;
		}

		JsonDocument get_cfg(const bool def) {
			return def ? m_def_cfg : m_user_cfg;
		}

	private:
		bool load_user_file() {
			if (!LITTLEFS.exists(k_user_file)) {
				DBG(msg::err, "cfg::load_user_file: can't find user config\n");
				return false;
			}

			auto file = LITTLEFS.open(k_user_file, FILE_READ);
			if (!file) {
				DBG(msg::err, "cfg::load_user_file: can't open user config\n");
				return false;
			}

			JsonDocument json{};
			if (const auto error = deserializeJson(json, file)) {
				DBG(msg::err, "cfg::load_user_file: failed to parse user config, error:\n");
				Serial.println(error.c_str());
				return false;
			}
			file.close();

			m_user_cfg = json;
			return true;
		}

		void load_def_file() {
			m_def_cfg[ "wifi_ssid" ] = "espwfsm";
			m_def_cfg[ "wifi_password" ] = "iforgotthepassword";

			m_def_cfg[ "display_brightness" ] = 100;

			m_def_cfg[ "led_clr_r" ] = 255.f;
			m_def_cfg[ "led_clr_g" ] = 255.f;
			m_def_cfg[ "led_clr_b" ] = 255.f;
			m_def_cfg[ "led_clr_a" ] = 1023;

			m_def_cfg[ "notice_timeout" ] = 15;
			m_def_cfg[ "notice_max_cnt" ] = 3;
		}

		void apply_def_file() {
			std::size_t applied_cnt{};

			for (JsonPair kv : m_def_cfg.as< JsonObject >()) {
				const char* key = kv.key().c_str();
				if (m_user_cfg[ key ].isNull()) {
					m_user_cfg[ key ] = kv.value();
					applied_cnt++;
					m_is_edited = true;
				}
			}

			if (applied_cnt > 0u)
				save_file();
		}

		void notify_observers(const cfg_key& key, const json_value& value) {
			const auto it = m_observers.find(key);
			if (it == m_observers.end())
				return;

			for (const auto& callback : it->second)
				callback(key, value);
		}
	};

	auto& g_cfg_mngr = c_cfg_manager::instance();
}
