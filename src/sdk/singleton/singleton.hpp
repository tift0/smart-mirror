#pragma once

template < typename _ty >
struct singleton_t {
	static _ty& instance() {
		static _ty ret;
		return ret;
	}

protected:
	singleton_t() = default;
	~singleton_t() = default;

	singleton_t(const singleton_t&) = delete;
	singleton_t& operator =(const singleton_t&) = delete;

	singleton_t(singleton_t&&) = delete;
	singleton_t& operator =(singleton_t&&) = delete;
};
