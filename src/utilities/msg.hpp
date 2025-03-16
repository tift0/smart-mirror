//
// created by tift0 on 15.03.2025.
//

#pragma once

#include <HardwareSerial.h>

namespace msg {
	enum e_msg { warn, err, inf, pos, neg, send, recv };

	class c_msg {
		static std::string get_prefix( e_msg type ) {
			std::string prefix{};
			switch ( type ) {
				case warn:
					prefix = "[*] ";
				break;
				case err:
					prefix = "[!] ";
				break;
				case inf:
					prefix = "[?] ";
				break;
				case pos:
					prefix = "[+] ";
				break;
				case neg:
					prefix = "[-] ";
				break;
				case send:
					prefix = "[>] ";
				break;
				case recv:
					prefix = "[<] ";
				break;
			}
			return prefix;
		}
	public:
		static void push( const e_msg type, const std::string& msg ) {
			const std::string result =
				get_prefix( type )
								.append( msg )
								.append( "\n" );

			Serial.print( result.c_str( ) );
		}
	};
}

#define DBG( prefix, txt ) msg::c_msg::push( prefix, txt )
