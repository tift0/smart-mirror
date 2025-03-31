#pragma once

#include <HardwareSerial.h>

namespace msg {
	enum e_msg { none, warn, err, inf, pos, neg, send, recv };

	class c_message {
		static std::string get_prefix( const e_msg type ) {
			switch ( type ) {
				case warn:
					return "[*] ";
				case err:
					return "[!] ";
				case inf:
					return "[?] ";
				case pos:
					return "[+] ";
				case neg:
					return "[-] ";
				case send:
					return "[>] ";
				case recv:
					return "[<] ";
				case none:
				default:
					return "";
			}
		}

	public:
		static void push( const e_msg type, const std::string& str ) {
			const std::string result =
				get_prefix( type )
								.append( str );

			Serial.print( result.c_str( ) );
		}

		static void push( const e_msg type, const StringSumHelper& str ) {
			push( type, std::string( str.c_str( ) ) );
		}

		static void push( const e_msg type, const char* str ) {
			push( type, std::string( str ) );
		}
	};
}

#define DBG( prefix, txt ) msg::c_message::push( prefix, txt )
