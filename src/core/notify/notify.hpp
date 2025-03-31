#pragma once

#include <utility>
#include <vector>

/*
 * @todo:
 *		> design
 *		> implementation of renderer
 *		> smooth fade-out after 15 seconds
 */
namespace core {
	/* it's just a PoC, it's not the final version */
	class c_notice_mngr {
	private:
		struct notice_data_t {
			String m_title{}, m_msg{}, m_time{};

			explicit notice_data_t( String title, String msg, String time )
				: m_title( std::move( title ) ), m_msg( std::move( msg ) ), m_time( std::move( time ) ) {
			};
		};

		// cur - for our purposes, all - for debug purposes
		std::vector< notice_data_t > m_cur_notice{}, m_all_notices{};

	public:
		void process( const String& title, const String& msg, const String& time ) {
			m_cur_notice.emplace_back( title, msg, time );

			m_all_notices.emplace_back( title, msg, time );
		}

		void handle( ) {
			for ( const auto& i : m_cur_notice )
				Serial.println(
					"> received new msg from: " + i.m_title
					+ "\n  text: " + i.m_msg
					+ "\n  at: " + i.m_time
					+ "\n"
				);

			m_cur_notice.clear( );
		}

		std::vector< notice_data_t > get_notices( ) const {
			return m_all_notices;
		}
    };

	c_notice_mngr g_notice_mngr{};
}
