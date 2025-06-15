#pragma once

#include "core/draw/draw.hpp"

#include "sdk/anim/anim.hpp"

namespace view {
	class c_display : public singleton_t< c_display > {
	private:
		bool m_is_initialized{};

		void show_date(const math::vec2_t< int > pos) {
			static std::array< char, 6 > time_str{ "00:00" };

			const auto now = time(nullptr);
			if (const struct tm* info = localtime(&now))
				snprintf(
					time_str.data(), time_str.size(), "%02d:%02d",
					info->tm_hour, info->tm_min
				);
			else
				snprintf(time_str.data(), time_str.size(), "--:--");

			core::g_draw.draw_text(pos, time_str.data(), core::e_clr::white, 1, core::e_align::center);
		}

		void show_charge(const math::vec2_t< int > pos) {
			constexpr auto k_size = 15;
			const auto     percentage = 0.75f;/*view::g_battery.percent(view::g_battery.voltage());*/

			core::g_draw.draw_rect(pos, { static_cast< int >(k_size * percentage), 7 }, core::e_clr::white);

			// @todo: +is_charging
		}

		void show_notice(math::vec2_t< int > pos) {
			const auto notices = core::g_notice_mngr.get_active_notices();
			if (notices.empty())
				return;

			// @todo: split the str len by screen limit
			int padding = 34;
			for (auto& notice : notices) {
				if (notice.m_is_active) {
					constexpr int k_max_length = 20;
					String        str = notice.m_app + " -> " + notice.m_msg;
					String        disp_str = str.length() > k_max_length
															? str.substring(0u, k_max_length - 3u) + "..." : str;

					core::g_draw.draw_text(
						{ pos.x(), pos.y() + padding }, disp_str, core::e_clr::white
					);
					padding += 17;
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

			auto screen_size = core::g_draw.screen_size();

			show_date({ screen_size.x() / 2, 1 });

			show_charge({ screen_size.x() - 17, 1 });

			show_notice({ 1, screen_size.y() / 2 });
		}
	};

	auto& g_display = c_display::instance();
}
