#pragma once

#include <ArduinoJson.h>
#include <WebServer.h>
#include <WiFiMulti.h>
#include <ESPping.h>

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

		String			m_ssid{ "DDosNIGGAS" }, m_password{ "4xyypCFo" };
		std::uint8_t	m_reconnect_attempts{};

		std::unique_ptr< WebServer >	m_server;
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

			bool is_changed{};
			for (JsonPair kv : json.as< JsonObject >()) {
				const auto key = kv.key().c_str();
				g_cfg_mngr.set(key, kv.value());
				is_changed = true;
			}

			is_changed ? g_cfg_mngr.save_file() : 0;

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

		void checkConnection() {
			switch(WiFi.status()) {
				case WL_CONNECTED:
					DBG(msg::pos, "wifi: connected\n");
					Serial.println(WiFi.SSID());
					Serial.println(WiFi.localIP());

					DBG(msg::inf, "ping google.com - ");
					if (Ping.ping("google.com"))
						DBG(msg::none, "ok\n");
					else
						DBG(msg::none, "fail\n");

					break;

				case WL_CONNECT_FAILED:
					DBG(msg::err, "wifi: wrong password or smt\n");
					break;

				default:
					DBG(msg::err, "wifi: unknown\n");
			}
		}

		void connect() const {
			if (m_ssid.length() == 0 && m_password.length() == 0) {
				DBG(msg::err, "wifi::connect: received empty auth data\n");

			if (WiFi.status() == WL_CONNECTED)
				return;

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
					reconnect();
				}
			);

			g_cfg_mngr.add_observer(
				"wifi_password", [ & ](const std::string& key, const JsonVariant& value) {
					m_password = value.as< String >();
					reconnect();
				}
			);
		}

		void setup_wifi() {
			WiFi.mode(WIFI_AP_STA);

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

			if (!WiFi.enableAP(true)) {
				DBG(msg::err, "failed to enable AP\n");
				return;
			}

			esp_netif_t* ap_netif = esp_netif_get_handle_from_ifkey("WIFI_AP_DEF");
			esp_netif_t* sta_netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");

			if (ap_netif && sta_netif) {
				esp_netif_ip_info_t ap_ip_info{};
				ap_ip_info.ip.addr = local_ip;
				ap_ip_info.netmask.addr = subnet;
				ap_ip_info.gw.addr = gateway;
				esp_netif_set_ip_info(ap_netif, &ap_ip_info);

				esp_netif_ip_info_t sta_ip_info{};
				esp_netif_get_ip_info(sta_netif, &sta_ip_info);

				esp_netif_ip_info_t route_info{};
				route_info.ip.addr = 0;
				route_info.netmask.addr = 0;
				route_info.gw.addr = sta_ip_info.gw.addr;
				esp_netif_set_ip_info(ap_netif, &route_info);
			}

			DBG(msg::inf, "esp ip address: ");
			Serial.println(WiFi.softAPIP());

			WiFi.setSleep(false);

			connect();
		}

		void setup_server() {
			reg_http_handlers();
			m_server->begin();
		}

		void handle() {
			m_server->handleClient();

			static std::uint32_t last_update{};
			constexpr static auto k_update_delay = 10000u;

			const auto cur_time = millis();
			if (cur_time - last_update >= k_update_delay) {
				DBG(msg::inf, "AP status: ");
				Serial.printf("connected clients: %d\n", WiFi.softAPgetStationNum());
				Serial.printf("AP ip: %s\n", WiFi.softAPIP().toString().c_str());
				Serial.printf("AP gateway: %s\n", WiFi.softAPNetworkID().toString().c_str());

				if (WiFi.status() == WL_CONNECTED) {
					Serial.printf("STA ip: %s\n", WiFi.localIP().toString().c_str());
					Serial.printf("STA gateway: %s\n", WiFi.gatewayIP().toString().c_str());
					Serial.printf("STA dns: %s\n", WiFi.dnsIP().toString().c_str());
					checkConnection();
				}

				last_update = cur_time;
			}
		}
	};

	auto& g_wifi = c_wifi::instance();
}
