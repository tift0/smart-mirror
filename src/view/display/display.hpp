#pragma once

namespace view {
	class c_display : public singleton_t< c_display > {
	private:
		bool m_is_initialized{};

		void show_date(const math::vec2_t< std::int16_t > pos) {
			DBG(msg::inf, "show_date: starting...\n");

			if (!m_is_initialized) {
				DBG(msg::err, "show_date: display not initialized\n");
				return;
			}

			if (!core::g_renderer.is_valid()) {
				DBG(msg::err, "show_date: renderer not valid\n");
				return;
			}

			DBG(msg::inf, "show_date: creating time string...\n");
			static char time_str[9]{};
			const std::uint32_t cur_time = millis();

			DBG(msg::inf, "show_date: formatting time...\n");
			snprintf(time_str, sizeof(time_str), "%02d:%02d",
				(cur_time / 3600000) % 24,
				(cur_time / 60000) % 60
			);

			DBG(msg::inf, "show_date: drawing text...\n");
			core::g_renderer.draw_text(pos, time_str);

			DBG(msg::inf, "show_date: completed\n");
		}

		void show_charge(const math::vec2_t< std::int16_t > pos) {
			constexpr auto k_size = 15;
			const auto     percentage = 0.25f; //view::g_battery.percent(view::g_battery.voltage());

			//core::g_renderer.draw_text( {10, 10}, std::to_string(percentage));

			core::g_renderer.draw_rect(pos, { static_cast< std::int16_t >(k_size * percentage), 7 });

			// @todo: +is_charging - done!
                        // const bool is_charging = view::g_battery.is_charging();

                        // if is_charging {
			// 	core::g_renderer.draw_text( {10, 10}, "Charging");
			// }
		}

		void show_notice(const math::vec2_t< std::int16_t > pos) {
			const auto notices = core::g_notice_mngr.get_active_notices();
			if (notices.empty())
				return;


			// @todo: split the str len by screen limit - done!
                        constexpr uint16_t max_length = 20;
			std::uint16_t padding = 34;
			for (auto& notice : notices) {
				if (notice.m_is_active) {
					String full_str = notice.m_app + ": " + notice.m_title + " " + notice.m_msg;
                                        String disp_str = full_str.length() > max_length
                                                          ? full_str.substring(0, max_length - 3) + "..."
                                                          : full_str;

					core::g_renderer.draw_text(
						{ 0, padding }, disp_str
					);
					padding += 17;


				}
			}

			// @todo: +title/app - done!
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

			show_notice({ 0, static_cast< short >(screen_size.y() / 2) });
		}
	};

	auto& g_display = c_display::instance();
}
