#pragma once

#include <unordered_map>

#include <ArduinoJson.h>
#include <FS.h>
#include <LittleFS.h>

/*
 * @todo:
 *      > mutex
 */
namespace core {
	class c_cfg_manager : public singleton_t< c_cfg_manager > {
		using cfg_key = std::string;
		using json_value = ArduinoJson::JsonVariant;
		using change_callback = std::function< void(const cfg_key&, const json_value&) >;

	private:
		constexpr static auto			k_def_file = "/def_cfg.json",
										k_user_file = "/user_cfg.json";
		constexpr static std::size_t	k_capacity = 4096u;

		ArduinoJson::StaticJsonDocument< k_capacity > m_def_cfg{}, m_user_cfg{};

		bool m_is_edited{};

		std::unordered_map< cfg_key, std::vector< change_callback > > m_observers{};

	public:
		[[noreturn]] void process() {
			if (!LITTLEFS.begin(false)) {
				DBG(msg::warn, "littlefs mount failed, formatting...\n");
				if (LITTLEFS.format()) {
					DBG(msg::pos, "format successful, retrying mount...\n");
					if (!LITTLEFS.begin(false)) {
						DBG(msg::warn, "mount still failed after format\n");
						while (true);
					}
				}
				else {
					DBG(msg::neg, "format failed\n");
					while (true);
				}
			}
			else
				DBG(msg::pos, "littlefs mounted successfully\n");


			load_def_file();

			if (!load_user_file())
				DBG(msg::inf, "user config not found, using defaults\n");

			apply_def_file();
		}

		bool set(const cfg_key& key, const JsonVariant& value) {
			m_user_cfg[ key ] = value;
			m_is_edited = true;

			notify_observers(key, value);

			return true;
		}

		// @todo: doc[ key ].is<T>()
		[[nodiscard]] json_value get(const cfg_key& key) {
			json_value result{};

			if (m_user_cfg.containsKey(key))
				result = m_user_cfg[ key ];
			else if (m_def_cfg.containsKey(key))
				result = m_def_cfg[ key ];

			return result;
		}

		void add_observer(const cfg_key& key, change_callback callback) {
			m_observers[ key ].push_back(std::move(callback));
		}

		bool save_file() {
			if (!m_is_edited)
				return true;

			File file = LITTLEFS.open(k_user_file, FILE_WRITE);
			if (!file) {
				DBG(msg::err, "can't open user config\n");
				return false;
			}

			ArduinoJson::StaticJsonDocument< k_capacity > json{};
			for (JsonPair kv : m_user_cfg.as< JsonObject >())
				json[ kv.key() ] = kv.value();

			if (serializeJson(json, file) == 0) {
				DBG(msg::err, "failed to write json to file\n");
				file.close();
				return false;
			}

			file.close();
			m_is_edited = false;
			return true;
		}

	private:
		bool load_user_file() {
			if (!LITTLEFS.exists(k_user_file)) {
				DBG(msg::err, "can't find user config\n");
				return false;
			}

			auto file = LITTLEFS.open(k_user_file, FILE_READ);
			if (!file) {
				DBG(msg::err, "can't open user config\n");
				return false;
			}

			ArduinoJson::StaticJsonDocument< k_capacity > json{};
			if (const auto error = deserializeJson(json, file)) {
				DBG(msg::err, "failed to parse user config, error:\n");
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
			m_def_cfg[ "display_rotation" ] = 0;

			m_def_cfg[ "led_color_r" ] = 255;
			m_def_cfg[ "led_color_g" ] = 255;
			m_def_cfg[ "led_color_b" ] = 255;
			m_def_cfg[ "led_color_a" ] = 255;

			m_def_cfg[ "notification_timeout" ] = 15;
			m_def_cfg[ "notification_max_count" ] = 3;
		}

		void apply_def_file() {
			std::size_t applied_cnt{};

			for (JsonPair kv : m_def_cfg.as< JsonObject >()) {
				const char* key = kv.key().c_str();
				if (!m_user_cfg.containsKey(key)) {
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
