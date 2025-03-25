//
// created by tift0 on 15.03.2025.
//

#pragma once

#include <HardwareSerial.h>

namespace msg {
	enum e_msg { none, warn, err, inf, pos, neg, send, recv };

	class c_msg {
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
		static void push( const e_msg type, const std::string& msg ) {
			const std::string result =
				get_prefix( type )
								.append( msg );

			Serial.print( result.c_str( ) );
		}
	};
}

#define DBG( prefix, txt ) msg::c_msg::push( prefix, txt )
