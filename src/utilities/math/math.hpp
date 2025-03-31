#pragma once

// so primitive
namespace math {
	struct vec2_t {
		std::uint16_t _x{}, _y{};

	public:
		vec2_t( ) = default;
		vec2_t( std::uint16_t x, std::uint16_t y ) : _x(x), _y(y) {};

		std::uint16_t x( ) { return _x; }
		std::uint16_t y( ) { return _y; }

		bool operator==( const vec2_t &other ) const { return _x == other._x && _y == other._y; }
		bool operator!=( const vec2_t &other ) const { return !( *this == other ); }
	};
}
