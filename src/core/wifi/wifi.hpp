#pragma once

#include <ArduinoJson.h>
#include <WebServer.h>
#include <WiFiMulti.h>
#include <ArduinoHttpClient.h>

#include "core/notice/notice.hpp"

#include "core/cfg/cfg.hpp"

namespace core {
	struct recv_data_t {
	private:
		String m_title{}, m_msg{}, m_app{};

	public:
		recv_data_t() = default;

		explicit recv_data_t(const std::tuple< String, String, String >& _tuple)
			: m_title(std::get< 0 >(_tuple)),
			  m_msg(std::get< 1 >(_tuple)),
			  m_app(std::get< 2 >(_tuple)) {
		}

		/* title / message / app */
		auto get() const noexcept -> std::tuple< const String&, const String&, const String& > {
			return std::tie(m_title, m_msg, m_app);
		}
	} m_recv_data{};

	class c_wifi : public singleton_t< c_wifi > {
	private:
		static constexpr std::uint8_t	k_max_reconnect_attempts = 5u;
		static constexpr std::uint16_t	k_def_port = 80u;

		String			m_ssid{}, m_password{};
		std::uint8_t	m_reconnect_attempts{};

		std::unique_ptr< WebServer >	m_server;

	private:
		void handle_notice_post() {
			DBG(msg::recv, "request received at /post:\n");
			for (int i{}; i < m_server->headers(); i++)
				Serial.printf("Header %d: %s: %s\n", i, m_server->headerName(i).c_str(), m_server->header(i).c_str());

			DBG(msg::inf, "raw data length: ");
			Serial.println(m_server->arg("plain").length());
			DBG(msg::inf, "raw data: ");
			Serial.println(m_server->arg("plain"));

			/* wrong method */
			if (m_server->method() != HTTP_POST) {
				DBG(msg::err, "notice::handle_notice_post: wrong method\n");
				m_server->send(405, "application/json", R"({"error": "method not allowed"})");
				return;
			}

			/* no available data */
			if (!m_server->hasArg("plain")) {
				DBG(msg::err, "notice::handle_notice_post: no data\n");
				m_server->send(400, "application/json", R"({"error": "no data"})");
				return;
			}

			const String body = m_server->arg("plain");
			
			/* the received data is too large */
			if (body.length() > 512) {
				DBG(msg::err, "notice::handle_notice_post: data too large\n");
				m_server->send(400, "application/json", R"({"error": "data too large"})");
				return;
			}

			JsonDocument json{};
			const auto error = deserializeJson(json, body);
			/* invalid json */
			if (error) {
				DBG(msg::err, "notice::handle_notice_post: invalid json, error: ");
				Serial.println(error.c_str());
				m_server->send(400, "application/json", R"({"error": "invalid json"})");
				return;
			}

			/* fill struct data */
			{
				if (!json.containsKey("title") || !json.containsKey("message") || !json.containsKey("application")) {
					DBG(msg::err, "notice::handle_notice_post: missing required fields\n");
					m_server->send(400, "application/json", R"({"error": "missing required fields"})");
					return;
				}

				const auto	title = json["title"].as<String>(),
							msg = json["message"].as<String>(),
							app = json["application"].as<String>();

				m_recv_data = recv_data_t(
					std::make_tuple(title, msg, app)
				);

				g_notice_mngr.process(title, msg, app);
			}

			const auto response = R"({"status": "success"})";
			m_server->send(200, "application/json", response);
		}

		void handle_cfg_post() {
			DBG(msg::recv, "request received at /post_cfg:\n");
			for (int i{}; i < m_server->headers(); i++)
				Serial.printf("%s: %s\n", m_server->headerName(i).c_str(), m_server->header(i).c_str());

			/* wrong method */
			if (m_server->method() != HTTP_POST) {
				DBG(msg::err, "cfg::handle_cfg_post: wrong method\n");
				m_server->send(405, "application/json", R"({"error": "method not allowed"})");
				return;
			}

			/* no available data */
			if (!m_server->hasArg("plain")) {
				DBG(msg::err, "cfg::handle_cfg_post: no data\n");
				m_server->send(400, "application/json", R"({"error": "no data"})");
				return;
			}

			const String body = m_server->arg("plain");
			DBG(msg::inf, "body: ");
			Serial.println(body);

			/* the received data is too large */
			if (body.length() > 512) {
				DBG(msg::err, "cfg::handle_cfg_post: data too large\n");
				m_server->send(400, "application/json", R"({"error": "data too large"})");
				return;
			}

			JsonDocument json{};
			const auto error = deserializeJson(json, body);
			/* invalid json */
			if (error) {
				DBG(msg::err, "cfg::handle_cfg_post: invalid json, error: ");
				Serial.println(error.c_str());
				m_server->send(400, "application/json", R"({"error": "invalid json"})");
				return;
			}

			bool is_changed{};
			for (JsonPair kv : json.as< JsonObject >()) {
				const auto key = kv.key().c_str();
				const auto value = kv.value();
				DBG(msg::inf, "setting config: ");
				Serial.print(key);
				Serial.print(" = ");
				Serial.println(value.as< String >());
				
				g_cfg_mngr.set(key, value);
				is_changed = true;
			}

			if (is_changed) {
				DBG(msg::inf, "cfg::handle_cfg_post: config changed, saving...\n");
				g_cfg_mngr.save_file();
			}

			const auto response = R"({"status": "success"})";
			m_server->send(200, "application/json", response);
		}

		void handle_cfg_get() {
			DBG(msg::recv, "request received at /cfg:\n");
			for (int i{}; i < m_server->headers(); i++)
				Serial.printf("%s: %s\n", m_server->headerName(i).c_str(), m_server->header(i).c_str());

			auto	def_obj = g_cfg_mngr.get_cfg(true),
					user_obj = g_cfg_mngr.get_cfg(false);

			JsonDocument json{};
			for (JsonPair kv : def_obj.as< JsonObject >()) {
				const char* key = kv.key().c_str();
				!user_obj[key].isNull()
					? json[ key ] = user_obj[ key ]
					: json[ key ] = kv.value();
			}

			String str{};
			if (serializeJson(json, str) == 0) {
				m_server->send(400, "application/json", R"({"error": "serialization failed"})");
				return;
			}

			m_server->send(200, "application/json", str);
		}

		void handle_time_post() {
			DBG(msg::recv, "post request received at /time\n");
			
			for (int i{}; i < m_server->headers(); i++)
				Serial.printf("header %d: %s: %s\n", i, m_server->headerName(i).c_str(), m_server->header(i).c_str());

			if (m_server->hasArg("plain")) {
				DBG(msg::inf, "raw data: ");
				Serial.println(m_server->arg("plain"));
			} else {
				DBG(msg::inf, "no raw data received\n");
				m_server->send(400, "application/json", R"({"error": "no data"})");
				return;
			}

			JsonDocument json;
			if (deserializeJson(json, m_server->arg("plain")) != DeserializationError::Ok) {
				m_server->send(400, "application/json", R"({"error": "invalid json"})");
				return;
			}

			if (!json.containsKey("timestamp")) {
				m_server->send(400, "application/json", R"({"error": "missing 'timestamp'"})");
				return;
			}

			const time_t t = json[ "timestamp" ];
			const timeval now = { .tv_sec = t };
			settimeofday(&now, nullptr);
			configTime(5 * 3600, 0, nullptr, nullptr);

			DBG(msg::pos, "time updated: ");
			Serial.println(String(t));

			m_server->send(200, "application/json", R"({"status": "success"})");
		}

		void reg_http_handlers() {
			DBG(msg::inf, "registering http handlers...\n");

			m_server->on("/", HTTP_GET, [ & ] {
				DBG(msg::recv, "root endpoint called\n");
				m_server->send(200, "text/plain", "Server is working!");
			});

			m_server->on(
				"/post", HTTP_POST, [ & ] { handle_notice_post(); }
			);

			m_server->on(
				"/post_cfg", HTTP_POST, [ & ] { handle_cfg_post(); }
			);

			m_server->on(
				"/time", HTTP_POST, [ & ] { handle_time_post(); }
			);

			m_server->on(
				"/cfg", HTTP_GET, [ & ] { handle_cfg_get(); }
			);

			m_server->onNotFound([ & ] {
				DBG(msg::recv, "404 - not found: ");
				Serial.println(m_server->uri());
				DBG(msg::inf, "method: ");
				Serial.println(m_server->method() == HTTP_GET ? "GET" : 
							 m_server->method() == HTTP_POST ? "POST" : "OTHER");
				
				for (int i{}; i < m_server->headers(); i++)
					Serial.printf("header %d: %s: %s\n", i, m_server->headerName(i).c_str(), m_server->header(i).c_str());

				m_server->send(404, "text/plain", "Not found");
			});

			DBG(msg::pos, "http handlers registered\n");
		}

	public:
		c_wifi() {
			m_server = std::unique_ptr<WebServer>(new WebServer(k_def_port));
			if (!m_server) {
				DBG(msg::err, "wifi::c_wifi: webserver alloc failed\n");
				esp_restart();
			}
		}

		void process() {
			setup_observers();
			setup_wifi();
			setup_server();
		}

		void setup_observers() {
			g_cfg_mngr.add_observer(
				"wifi_ssid", [ & ](const std::string& key, const JsonVariant& value) {
					m_ssid = value.as< String >();
					DBG(msg::warn, "wifi_ssid updated, restarting...\n");
					esp_restart();
				}
			);

			g_cfg_mngr.add_observer(
				"wifi_password", [ & ](const std::string& key, const JsonVariant& value) {
					m_password = value.as< String >();
					DBG(msg::warn, "wifi_password updated, restarting...\n");
					esp_restart();
				}
			);
		}

		void setup_wifi() {
			WiFi.mode(WIFI_AP);

			const IPAddress local_ip(192, 168, 4, 1),
							gateway(192, 168, 4, 1),
							subnet(255, 255, 255, 0);

			if (!WiFi.softAPConfig(local_ip, gateway, subnet)) {
				DBG(msg::err, "AP config failed\n");
				return;
			}

			if (!WiFi.softAP("espwfsm", "iforgotthepassword")) {
				DBG(msg::err, "AP setup failed\n");
				return;
			}

			DBG(msg::inf, "esp ip address: ");
			Serial.println(WiFi.softAPIP());

			WiFi.setSleep(false);
		}

		void setup_server() {
			DBG(msg::inf, "Setting up server...\n");
			reg_http_handlers();
			m_server->begin();
			DBG(msg::pos, "Server started on port ");
			Serial.println(k_def_port);
		}

		void handle() const {
			m_server->handleClient();

			static unsigned long last_check = 0;
			if (millis() - last_check > 10000) {  // every 10 seconds
				DBG(msg::inf, "connected stations: ");
				Serial.println(WiFi.softAPgetStationNum());
				last_check = millis();
			}
		}
	};

	auto& g_wifi = c_wifi::instance();
}
