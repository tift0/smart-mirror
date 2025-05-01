#pragma once

#include <algorithm>

namespace math {
	struct clr_t {
	private:
		union {
			struct {
				std::uint8_t m_r{},
				             m_g{},
				             m_b{};
			};

			std::uint32_t m_rgb{};
		};

		// dat nigga standard don't got this
		template < class _ty >
		constexpr const _ty& clamp(const _ty& value, const _ty& min, const _ty& max) {
			return (value < min) ? min : (max < value) ? max : value;
		}

	public:
		clr_t() = default;

		clr_t(const int r, const int g, const int b)
			: m_r{ static_cast< std::uint8_t >(clamp(r, 0, 255)) },
			  m_g{ static_cast< std::uint8_t >(clamp(g, 0, 255)) },
			  m_b{ static_cast< std::uint8_t >(clamp(b, 0, 255)) } {
		}

		clr_t(const float r, const float g, const float b)
			: m_r{ static_cast< std::uint8_t >(clamp(r, 0.0f, 1.0f) * 255.0f) },
			  m_g{ static_cast< std::uint8_t >(clamp(g, 0.0f, 1.0f) * 255.0f) },
			  m_b{ static_cast< std::uint8_t >(clamp(b, 0.0f, 1.0f) * 255.0f) } {
		}

		explicit clr_t(const std::uint32_t rgb) : m_rgb{ rgb } {
		}

		static clr_t from_hsb(const float hue, const float saturation, const float brightness) {
			const auto h = hue == 1.f ? 0 : hue * 6.f;
			const auto f = h - static_cast< int >(h);
			const auto p = brightness * (1.f - saturation);
			const auto q = brightness * (1.f - saturation * f);
			const auto t = brightness * (1.f - (saturation * (1.f - f)));

			if (h < 1.f)
				return { brightness, t, p } ;
			else if (h < 2.f)
				return { q, brightness, p };
			else if (h < 3.f)
				return { p, brightness, t };
			else if (h < 4.f)
				return { p, q, brightness };
			else if (h < 5.f)
				return { t, p, brightness };

			return { brightness, p, q };
		}

		float get_hue() const {
			const auto red = r() / 255.f;
			const auto green = g() / 255.f;
			const auto blue = b() / 255.f;

			const auto max = std::max< float >({ red, green, blue });
			const auto min = std::min< float >({ red, green, blue });

			if (max == min)
				return 0.f;

			const auto delta = max - min;

			float hue{};
			if (max == red)
				hue = (green - blue) / delta;
			else if (max == green)
				hue = 2.f + (blue - red) / delta;
			else
				hue = 4.f + (red - green) / delta;

			hue *= 60.f;

			if (hue < 0.f)
				hue += 360.f;

			return hue / 360.f;
		}

		float get_brightness() const {
			return std::max< float >({ r() / 255.f, g() / 255.f, b() / 255.f });
		}

		float get_saturation() const {
			const auto red = r() / 255.f;
			const auto green = g() / 255.f;
			const auto blue = b() / 255.f;

			const auto max = std::max< float >({ red, green, blue });
			const auto min = std::min< float >({ red, green, blue });

			const auto delta = max - min;

			if (max == 0.f)
				return delta;

			return delta / max;
		}

		void set_hue(const float hue) {
			const auto color = from_hsb(hue, get_saturation(), get_brightness());
			*this = color;
		}

		void set_saturation(const float saturation) {
			const auto color = from_hsb(get_hue(), saturation, get_brightness());
			*this = color;
		}

		void set_brightness(const float brightness) {
			const auto color = from_hsb(get_hue(), get_saturation(), brightness);
			*this = color;
		}

		float r() const { return m_r / 255.0f; }
		float g() const { return m_g / 255.0f; }
		float b() const { return m_b / 255.0f; }

		void r(const float value) { m_r = value; }
		void g(const float value) { m_g = value; }
		void b(const float value) { m_b = value; }

		std::uint32_t& rgb() { return m_rgb; }

		explicit operator std::uint32_t() const { return m_rgb; }
	};
}
