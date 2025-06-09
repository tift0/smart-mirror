#pragma once

#include <complex>

namespace sdk {
	class c_animation : public singleton_t< c_animation > {
	private:
		constexpr static std::uint8_t  k_speed = 5u;
		constexpr static std::uint8_t  k_duration = 25u;
		constexpr static std::uint16_t k_delay = k_duration / k_speed;

	public:
		std::uint16_t fade_out(const std::uint16_t clr) {
			std::uint16_t result{};

			for (std::int8_t i = k_speed; i >= 0; i--) {
				const std::uint16_t r = (clr >> 11) * i / k_speed,
									g = ((clr >> 5) & 0x3F) * i / k_speed,
									b = (clr & 0x1F) * i / k_speed;

				result = (r << 11) | (g << 5) | b;
			}

			return result;
		}

		std::uint16_t fade_in(const std::uint16_t clr) {
			std::uint16_t result{};

			for (std::uint8_t i{}; i < k_speed; i++) {
				const std::uint16_t r = (clr >> 11) * i / k_speed,
									g = ((clr >> 5) & 0x3F) * i / k_speed,
									b = (clr & 0x1F) * i / k_speed;

				result = (r << 11) | (g << 5) | b;
			}

			return result;
		}
	};

	auto& g_anim = c_animation::instance();
}
