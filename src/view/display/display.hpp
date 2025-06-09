#pragma once

#include "core/renderer/renderer.hpp"

#include "sdk/anim/anim.hpp"

namespace view {
	class c_display : public singleton_t< c_display > {
	private:
		bool m_is_initialized{};

		void show_date(const math::vec2_t< std::int16_t > pos) {
			static char         time_str[ 6 ] = "00:00";
			const std::uint32_t cur_time = millis();

			snprintf(time_str, sizeof(time_str), "%02d:%02d",
					 (cur_time / 3600000) % 24,
					 (cur_time / 60000) % 60
			);

			core::g_renderer.draw_text(pos, time_str, core::e_clr::white, 1, core::e_align::center);
		}

		void show_charge(const math::vec2_t< std::int16_t > pos) {
			constexpr auto k_size = 15;
			const auto     percentage = view::g_battery.percent(view::g_battery.voltage());

			core::g_renderer.draw_text( { 10, 20 }, String(percentage), core::e_clr::white);

			core::g_renderer.draw_rect(pos, { static_cast< std::int16_t >(k_size * percentage), 7 }, core::e_clr::white);

			// @todo: +is_charging
		}

		void show_notice(const math::vec2_t< std::int16_t > pos) {
			const auto notices = core::g_notice_mngr.get_active_notices();
			if (notices.empty())
				return;

			// @todo: split the str len by screen limit
			std::int16_t			padding = 34u;
			constexpr std::int16_t	k_max_length = 20;
			for (auto& notice : notices) {
				if (notice.m_is_active) {
					String					str = notice.m_app + ": " + notice.m_msg;
					String					disp_str = str.length() > k_max_length
															? str.substring(0u, k_max_length - 3u) + "..." : str;

					core::g_renderer.draw_text(
						{ 0u, padding }, disp_str, core::e_clr::white
					);
					padding += 17u;
				}
			}
		}

	public:
		void process() {
			m_is_initialized = true;
		}

		void handle() {
			if (!m_is_initialized)
				return;

			auto screen_size = core::g_renderer.screen_size();

			show_date({ static_cast< short >(screen_size.x() / 2), 1 });

			show_charge({ static_cast< short >(screen_size.x() - 17), 1 });

			show_notice({ 1, static_cast< short >(screen_size.y() / 2) });
		}
	};

	auto& g_display = c_display::instance();
}
