#pragma once

#include <ArduinoJson.h>
#include <WebServer.h>

#include "core/notice/notice.hpp"

#include "core/cfg/cfg.hpp"

/*
 * @todo 1:
 *		> 2 way connection
 *		  i.e. connection between: phone -> esp -> local wifi
 */
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

		std::unique_ptr< WebServer > m_server;

	private:
		void handle_notice_post() {
			DBG(msg::recv, "request received at /post:\n");
			for (int i = 0; i < m_server->headers(); i++)
				Serial.printf("%s: %s\n", m_server->headerName(i).c_str(), m_server->header(i).c_str());

			DBG(msg::inf, "body: ");
			Serial.println(m_server->arg("plain"));

			/* wrong method */
			if (m_server->method() != HTTP_POST) {
				m_server->send(405, "application/json", R"({"error": "method not allowed"})");
				return;
			}

			/* no available data */
			if (!m_server->hasArg("plain")) {
				m_server->send(400, "application/json", R"({"error": "no data"})");
				return;
			}

			JsonDocument json{};
			const auto body = m_server->arg("plain");
			/* the received data is too large */
			if (body.length() > 512) {
				m_server->send(400, "application/json", R"({"error": "data too large"})");
				return;
			}

			/* invalid json */
			if (const auto error = deserializeJson(json, body)) {
				m_server->send(400, "application/json", R"({"error": "invalid json"})");
				return;
			}

			/* fill struct data */
			{
				const auto	title = json["title"].as< String >(),
							msg = json["message"].as< String >(),
							app = json[ "application" ].as< String >();

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

			DBG(msg::inf, "body: ");
			Serial.println(m_server->arg("plain"));

			/* wrong method */
			if (m_server->method() != HTTP_POST) {
				m_server->send(405, "application/json", R"({"error": "method not allowed"})");
				return;
			}

			/* no available data */
			if (!m_server->hasArg("plain")) {
				m_server->send(400, "application/json", R"({"error": "no data"})");
				return;
			}

			JsonDocument json{};
			auto body = m_server->arg("plain");
			/* the received data is too large */
			if (body.length() > 512) {
				m_server->send(400, "application/json", R"({"error": "data too large"})");
				return;
			}

			/* invalid json */
			if (const auto error = deserializeJson(json, body)) {
				m_server->send(400, "application/json", R"({"error": "invalid json"})");
				return;
			}

			bool changed{};
			for (JsonPair kv : json.as< JsonObject >()) {
				const auto key = kv.key().c_str();
				g_cfg_mngr.set(key, kv.value());
				changed = true;
			}

			if (changed)
				g_cfg_mngr.save_file();

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
				m_server->send(500, "application/json", R"({"error": "serialization failed"})");
				return;
			}

			m_server->send(200, "application/json", str);
		}

		void reg_http_handlers() {
			m_server->on(
				"/post", HTTP_POST, [ & ] { handle_notice_post(); }
			);

			m_server->on(
				"/post_cfg", HTTP_POST, [ & ] { handle_cfg_post(); }
			);

			m_server->on(
				"/cfg", HTTP_GET, [ & ] { handle_cfg_get(); }
			);

			m_server->onNotFound(
				[ & ] {
					DBG(msg::recv, "request received for unknown endpoint\n");
					for (int i{}; i < m_server->headers(); i++)
						Serial.printf("%s: %s\n", m_server->headerName(i).c_str(), m_server->header(i).c_str());

					m_server->send(404, "text/plain", "not found");
				}
			);
		}

		/* @todo 1 */
		void connect() const {
			if (m_ssid.length() == 0 || m_password.length() == 0) {
				DBG(msg::err, "wifi::connect: received empty auth data\n");
				return;
			}

			WiFi.begin(
				m_ssid.c_str(),
				m_password.c_str()
			);

			DBG(msg::inf, "wifi::connect: connecting to wifi: ");
			Serial.println(m_ssid.c_str());
		}

		void reconnect() {
			if (m_reconnect_attempts >= k_max_reconnect_attempts) {
				DBG(msg::warn, "wifi::reconnect: max reconnect attempts reached\n");
				return;
			}

			m_reconnect_attempts++;
			Serial.printf(
				"wifi::reconnect: reconnecting to wifi (attempt %d/%d)...\n", m_reconnect_attempts, k_max_reconnect_attempts
			);

			connect();
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
					//reconnect();
				}
			);

			g_cfg_mngr.add_observer(
				"wifi_password", [ & ](const std::string& key, const JsonVariant& value) {
					m_password = value.as< String >();
					//reconnect();
				}
			);
		}

		void setup_wifi() {
			WiFi.softAP(
				"espwfsm",
				"iforgotthepassword"
			);

			DBG(msg::inf, "ip address: ");
			Serial.println(WiFi.softAPIP());
		}

		void setup_server() {
			reg_http_handlers();
			m_server->begin();
		}

		void handle() const {
			m_server->handleClient();
		}
	};

	auto& g_wifi = c_wifi::instance();
}
