#pragma once

#include <ArduinoJson.h>
#include <WebServer.h>

#include "core/notice/notice.hpp"

#include "core/cfg/cfg.hpp"

// https://www.youtube.com/shorts/hv7uPsPRqJ0
constexpr char g_html_page[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>data</title>
    <style>
        body { font-family: Arial, sans-serif; text-align: center; margin-top: 50px; }
        .container { max-width: 400px; margin: auto; padding: 20px; border: 1px solid #ddd; border-radius: 10px; }
        h2 { color: #007bff; }
        p { font-size: 20px; }
        .application { font-size: 14px; color: gray; }
    </style>
</head>
<body>
    <div class="container">
        <h2>data</h2>
        <p><strong>title:</strong> <span id="title">wait...</span></p>
        <p><strong>message:</strong> <span id="message">wait...</span></p>
        <p class="application">application: <span id="application">-</span></p>
    </div>

    <script>
        function updateData() {
            fetch('/data')
                .then(response => response.json())
                .then(data => {
                    document.getElementById('title').innerText = data.title;
                    document.getElementById('message').innerText = data.message;
                    document.getElementById('application').innerText = data.application;
                })
                .catch(error => console.error('Error fetching data:', error));
        }
        setInterval(updateData, 500);
        updateData();
    </script>
</body>
</html>
)rawliteral";

namespace core {
	struct recv_data_t {
	private:
		String m_title{}, m_msg{}, m_app{};

	public:
		recv_data_t() = default;

		/* title / message / app  */
		explicit recv_data_t(const std::tuple< String, String, String >& _tuple)
			: m_title(std::get< 0 >(_tuple)),
			  m_msg(std::get< 1 >(_tuple)),
			  m_app(std::get< 2 >(_tuple)) {
		}

		/* title / message / app / time */
		std::tuple< const String&, const String&, const String& > get() const noexcept {
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
		void handle_post_request() {
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

			ArduinoJson::JsonDocument json;
			auto body = m_server->arg("plain");
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
			try {
				m_recv_data = recv_data_t(
					std::make_tuple(
						json["title"].as< String >(),
						json["message"].as< String >(),
						json["application"].as< String >()
					)
				);

				{
					const auto [ title, msg, app ] = m_recv_data.get();
					g_notice_mngr.process(title, msg, app);
				}

				const auto response = R"({"status": "success"})";
				m_server->send(200, "application/json", response);
			} catch (const std::exception& e) {
				m_server->send(500, "application/json", R"({"error": "internal server error"})");
			}
		}

		void handle_root() const {
			m_server->send(200, "text/html", g_html_page);
		}

		void handle_data() {
			const auto [ title, msg, app ] = m_recv_data.get();

			ArduinoJson::JsonDocument json;
			json["title"] = title;
			json["message"] = msg;
			json["application"] = app;

			String str{};
			if (serializeJson(json, str) == 0) {
				m_server->send(500, "application/json", R"({"error": "serialization failed"})");
				return;
			}

			m_server->send(200, "application/json", str);
		}

		void reg_http_handlers() {
			m_server->on(
				"/", [ & ] { handle_root(); }
			);

			m_server->on(
				"/data", [ & ] { handle_data(); }
			);

			m_server->on(
				"/post", HTTP_POST, [ & ] { handle_post_request(); }
			);

			m_server->onNotFound(
				[ & ] {
					m_server->send(404, "text/plain", "not found");
				}
			);
		}

		void connect() const {
			if (m_ssid.length() == 0 || m_password.length() == 0) {
				Serial.println("Invalid WiFi credentials");
				return;
			}

			WiFi.begin(m_ssid.c_str(), m_password.c_str());

			DBG(msg::inf, "connecting to wifi: ");
			Serial.println(m_ssid.c_str());
		}

		void reconnect() {
			if (m_reconnect_attempts >= k_max_reconnect_attempts) {
				Serial.println("max reconnect attempts reached");
				return;
			}

			m_reconnect_attempts++;
			Serial.printf(
				"reconnecting to wifi (attempt %d/%d)...\n", m_reconnect_attempts, k_max_reconnect_attempts
			);

			connect();
		}

	public:
		c_wifi() {
			try {
				m_server = std::unique_ptr< WebServer >(new WebServer(k_def_port));
				if (!m_server)
					throw std::runtime_error("WebServer allocation failed");
			} catch (const std::bad_alloc& e) {
				DBG(msg::err, "memalloc failed -> c_wifi\n");
				esp_restart();
			} catch (const std::exception& e) {
				Serial.printf("failed to create webserver: %s\n", e.what());
				esp_restart();
			} catch (...) {
				DBG(msg::err, "unknown error during webserver creation\n");
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
				"wifi_ssid", [this](const std::string& key, const JsonVariant& value) {
					m_ssid = value.as<String>();
					reconnect();
				}
			);

			g_cfg_mngr.add_observer(
				"wifi_password", [this](const std::string& key, const JsonVariant& value) {
					m_password = value.as<String>();
					reconnect();
				}
			);
		}

		void setup_wifi() {
			WiFi.softAP(
				g_cfg_mngr.get("wifi_ssid").as< String >(),
				g_cfg_mngr.get("wifi_password").as< String >()
			);

			DBG(msg::inf, "ip address: ");
			Serial.println(WiFi.softAPIP());
		}

		void setup_server() {
			m_server->begin();
			reg_http_handlers();
		}

		void handle() const {
			m_server->handleClient();
		}
	};

	auto& g_wifi = c_wifi::instance();
}
