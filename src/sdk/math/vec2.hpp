#pragma once

// primitive
namespace math {
	template < typename _ty = std::uint16_t >
	struct vec2_t {
		_ty _x{}, _y{};

	public:
		vec2_t() = default;

		vec2_t(_ty x, _ty y) : _x(x), _y(y) {
		}

		_ty x() { return _x; }
		_ty y() { return _y; }

		bool operator ==(const vec2_t& other) const { return _x == other._x && _y == other._y; }
		bool operator !=(const vec2_t& other) const { return !(*this == other); }
	};
}
