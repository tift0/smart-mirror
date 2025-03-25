#pragma once

#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>
#include <WiFi.h>

namespace backend {
	/*
	 * @todo:
	 *		> check this (NOT TESTED ON 26.03)
	 *		> struct w/ received data
	 */
	class c_wifi {
		std::string		m_ssid{},
						m_password{};
		AsyncWebServer	m_server;
		std::string		m_title{}, m_msg{}, m_app{}, m_time{};

	public:
		explicit c_wifi(
			std::string ssid, std::string password, const std::int8_t port
		) : m_ssid( std::move( ssid ) ), m_password( std::move( password ) ), m_server( port ) {}

		void process( ) {
			WiFi.mode( WIFI_AP );
			WiFi.softAP( m_ssid.c_str( ), m_password.c_str( ) );

			DBG( msg::inf, "AP addr: " );
			Serial.println( WiFi.softAPIP( ) );

			m_server.on(
				"/notify", HTTP_POST,
				[ ]( AsyncWebServerRequest* req ) {
					if ( req->getParam( "body", false ) ) {
						DBG( msg::err, "no data received\n" );
						req->send( 400, "text/plain", "no data" );
						return;
					}

					const auto body_param = req->getParam( "body", true );
					String json = body_param->value( );

					JsonDocument doc{};
					if ( auto error = deserializeJson( doc, json ) ) {
						DBG( msg::err, "deserializeJson( ) failed\n" );
						req->send( 400, "text/plain", "bad json" );
					}

					auto	title = doc[ "title" ].as< String >( ),
							msg = doc[ "message" ].as< String >( ),
							app = doc[ "application" ].as< String >( ),
							timestamp = doc[ "timestamp" ].as< String >( );

					// look at @todo

					req->send( 200, "text/plain", "data received" );
				}
			);

			m_server.begin( );
			DBG( msg::pos, "server started\n" );
		}
	};
}
