#pragma once

#include <ArduinoJson.h>
#include <WebServer.h>

#include "core/notify/notify.hpp"

// https://www.youtube.com/shorts/hv7uPsPRqJ0
constexpr char html_page[ ] PROGMEM = R"rawliteral(
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
        .timestamp { font-size: 14px; color: gray; }
    </style>
</head>
<body>
    <div class="container">
        <h2>data</h2>
        <p><strong>title:</strong> <span id="title">wait...</span></p>
        <p><strong>message:</strong> <span id="message">wait...</span></p>
        <p class="timestamp">time: <span id="timestamp">-</span></p>
    </div>

    <script>
        function updateData() {
            fetch('/data')
                .then(response => response.json())
                .then(data => {
                    document.getElementById('title').innerText = data.title;
                    document.getElementById('message').innerText = data.message;
                    document.getElementById('timestamp').innerText = data.timestamp;
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
		String m_title{}, m_msg{}, m_app{}, m_timestamp{};

	public:
		recv_data_t( ) = default;

		/* title / message / app / time */
		explicit recv_data_t( const std::tuple< String, String, String, String >& _tuple )
			: m_title( std::get< 0 >( _tuple ) ),
				m_msg( std::get< 1 >( _tuple ) ),
				m_app( std::get< 2 >( _tuple ) ),
				m_timestamp( std::get< 3 >( _tuple ) ) {
		}

		/* title / message / app / time */
		std::tuple< const String&, const String&, const String&, const String& > get( ) const noexcept {
			return std::tie( m_title, m_msg, m_app, m_timestamp );
		}
	} m_recv_data{};

	class c_wifi {
	private:
		std::string m_ssid{}, m_password{};
		WebServer   m_server;

		void handle_post_request( ) {
			/* wrong method */
			if ( m_server.method( ) != HTTP_POST ) {
				m_server.send( 405, "application/json", R"({"error": "method not allowed"})" );
				return;
			}

			/* no available data */
			if ( !m_server.hasArg( "plain" ) ) {
				m_server.send( 400, "application/json", R"({"error": "no data"})" );
				return;
			}

			JsonDocument	json{};
			String			body = m_server.arg( "plain" );
			/* invalid json */
			if ( auto error = deserializeJson( json, body ) ) {
				m_server.send( 400, "application/json", R"({"error": "invalid json"})" );
				return;
			}

			/* fill struct data */
			m_recv_data =
				recv_data_t( std::make_tuple(
					json[ "title" ].as< String >( ),
					json[ "message" ].as< String >( ),
					json[ "application" ].as< String >( ),
					json[ "timestamp" ].as< String >( )
				)
			);

			{ /* debug info */
				const auto [ title, msg, app, time ] = m_recv_data.get( );

				g_notice_mngr.process( title, msg, time );

				DBG( msg::inf, "new msg received\n" );
				DBG( msg::recv, "title: " + title + "\n" );
				DBG( msg::recv, "message: " + msg + "\n" );
				DBG( msg::recv, "app: " + app + "\n" );
				DBG( msg::recv, "time: " + time + "\n" );
			}

			const auto response = R"({"status": "success"})";
			m_server.send( 200, "application/json", response );
		}

		void handle_root( ) {
			m_server.send( 200, "text/html", html_page );
		}

		void handle_data( ) {
			const auto [ title, msg, app, time ] = m_recv_data.get( );

			JsonDocument json{};
			json[ "title" ]			= title;
			json[ "message" ]		= msg;
			json[ "application" ]	= app;
			json[ "timestamp" ]		= time;

			String str{};
			serializeJson( json, str );
			m_server.send( 200, "application/json", str );
		}

	public:
		explicit c_wifi( std::string ssid, std::string password, const int port )
			: m_ssid( std::move( ssid ) ), m_password( std::move( password ) ), m_server( port ) {
		}

		void process( ) {
			WiFi.softAP( m_ssid.c_str( ), m_password.c_str( ) );

			Serial.println( );
			Serial.print( "ip address: " );
			Serial.println( WiFi.softAPIP( ) );

			m_server.on(
				"/", [ & ] { handle_root( ); }
			);

			m_server.on(
				"/data", [ & ] { handle_data( ); }
			);

			m_server.on(
				"/post", HTTP_POST, [ & ] { handle_post_request( ); }
			);

			m_server.begin( );
		}

		void handle( ) {
			m_server.handleClient( );
		}
	};

	c_wifi g_wifi( "espwfsm", "iforgotthepassword", 80 );
}
