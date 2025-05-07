#pragma once

namespace view {
	class c_display : public singleton_t< c_display > {
	private:
		void show_date() {
			// @todo: parse data
		}

		void show_charge(const math::vec2_t< std::int16_t > pos) {
			constexpr auto	k_size = 15;
			const auto		percentage = view::g_battery.percent(view::g_battery.voltage());

			core::g_renderer.draw_text( {10, 10}, std::to_string(percentage));

			core::g_renderer.draw_rect(pos, { static_cast< std::int16_t >(k_size * percentage), 7 });

			// @todo: +is_charging
		}

		void show_notice(const math::vec2_t< std::int16_t > pos) {
			const auto notices = core::g_notice_mngr.get_active_notices();
			if (notices.empty())
				return;

			// @todo: split the str by screen limit
			for (auto& notice : notices)
				core::g_renderer.draw_text(pos, notice.m_msg.c_str(), core::e_flags::none);

			// @todo: +title/app
		}

	public:
		void handle() {
			if (!core::g_renderer.is_valid())
				return;

			auto screen_size = core::g_renderer.screen_size();

			show_charge({ static_cast< short >(screen_size.x() - 17), 10 });
		}
	};

	auto& g_display = c_display::instance();
}
