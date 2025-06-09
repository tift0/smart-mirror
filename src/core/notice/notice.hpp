#pragma once

#include <mutex>
#include <queue>
#include <utility>

#include "sdk/singleton/singleton.hpp"

namespace core {
	class c_notice_mngr : public singleton_t< c_notice_mngr > {
	private:
		struct notice_data_t {
			String									m_title{}, m_msg{}, m_app{};
			std::chrono::steady_clock::time_point	m_creation_time{};
			bool									m_is_active{};

			notice_data_t() = default;

			notice_data_t(String  title, String msg, String app,
			              const std::chrono::steady_clock::time_point creation_time,
			              const bool is_active = true) noexcept
				: m_title(std::move(title)), m_msg(std::move(msg)), m_app(std::move(app)),
				  m_creation_time(creation_time), m_is_active(is_active) {
			}

			std::tuple
				< const std::chrono::time_point< std::chrono::_V2::steady_clock >&, const String&, const String&, const String& >
			tie() const noexcept {
				return std::tie(m_creation_time, m_title, m_msg, m_app);
			}

			bool operator<(const notice_data_t& other) const noexcept {
				return tie() < other.tie();
			}
		};

		constexpr static std::size_t	k_queue_size = 10u;
		constexpr static std::size_t	k_max_notices = 3u;
		constexpr static std::uint16_t	k_max_time = 15u; // in seconds
		constexpr static std::size_t	k_invalid_index = std::numeric_limits< std::size_t >::max();

		std::mutex 					m_mutex{};
		std::queue< notice_data_t > m_queue{};

		std::array< notice_data_t, k_max_notices >	m_active_notices{};
		std::vector< notice_data_t >				m_history_notices{};
		std::size_t									m_active_notice_cnt{};

	private:
		static std::chrono::steady_clock::time_point _millis() {
			return std::chrono::steady_clock::time_point{ std::chrono::milliseconds(millis()) };
		}

		void cleanup_old_notices() noexcept {
			std::lock_guard< std::mutex > lock(m_mutex);
			const auto now = _millis();

			for (auto& notice : m_active_notices) {
				if (notice.m_is_active &&
					std::chrono::duration_cast< std::chrono::seconds >(now - notice.m_creation_time).count() >= k_max_time
				) {
					notice.m_is_active = false;
					if (m_active_notice_cnt > 0)
						--m_active_notice_cnt;
				}
			}
		}

		std::size_t find_free_slot() const noexcept {
			for (std::size_t i{}; i < k_max_notices; i++)
				if (!m_active_notices[ i ].m_is_active)
					return i;

			return k_invalid_index;
		}

		std::size_t find_oldest_notice() const noexcept {
			if (m_active_notice_cnt == 0u)
				return k_invalid_index;

			std::size_t oldest_idx = k_invalid_index;
			auto		oldest_time = std::chrono::steady_clock::time_point::max();

			for (std::size_t i{}; i < k_max_notices; i++) {
				const auto& notice = m_active_notices[ i ];
				if (notice.m_is_active && notice.m_creation_time < oldest_time) {
					oldest_time = notice.m_creation_time;
					oldest_idx = i;
				}
			}

			return oldest_idx;
		}

	public:
		c_notice_mngr() {
			m_history_notices.reserve(50);
		}

		bool process(const String& title, const String& msg, const String& app) {
			if (title.length() == 0 || msg.length() == 0 || app.length() == 0) {
				DBG(msg::neg, "notice::process: invalid notice data\n");
				return false;
			}

			Serial.printf(
				"processing new notice: title='%s', msg='%s', app='%s'\n", title.c_str(), msg.c_str(), app.c_str()
			);

			notice_data_t notice(title, msg, app, _millis(), true);

			std::lock_guard< std::mutex > lock(m_mutex);
			if (m_queue.size() < k_queue_size) {
				m_queue.push(std::move(notice));
				DBG(msg::pos, "notice::process: notice added to the queue successfully\n");
				return true;
			}
			else {
				DBG(msg::neg, "notice::process: failed to add notice to the queue\n");
				return false;
			}
		}

		void handle() {
			cleanup_old_notices();

			{
				notice_data_t notice{};
				std::lock_guard< std::mutex > lock(m_mutex);

				while (!m_queue.empty()) {
					notice = m_queue.front();
					m_queue.pop();

					if (m_active_notice_cnt < k_max_notices) {
						const auto slot = find_free_slot();
						if (slot != k_invalid_index) {
							m_active_notices[ slot ] = notice;
							++m_active_notice_cnt;

							if (m_history_notices.size() < 50u)
								m_history_notices.push_back(notice);
						}
					}
					else {
						const auto oldest = find_oldest_notice();
						if (oldest != k_invalid_index) {
							m_active_notices[ oldest ] = notice;
							if (m_history_notices.size() < 50u)
								m_history_notices.push_back(notice);
						}
					}
				}
			}
		}

		[[nodiscard]] std::vector< notice_data_t > get_active_notices() {
			std::lock_guard< std::mutex > lock(m_mutex);
			std::vector< notice_data_t > result{};
			result.reserve(k_max_notices);

			for (const auto& notice : m_active_notices)
				if (notice.m_is_active)
					result.push_back(notice);

			return result;
		}

		[[nodiscard]] std::vector< notice_data_t > get_history_notices() {
			std::lock_guard< std::mutex > lock(m_mutex);
			return m_history_notices;
		}

		void clear_all_notices() noexcept {
			std::lock_guard< std::mutex > lock(m_mutex);
			for (auto& notice : m_active_notices)
				notice.m_is_active = false;

			m_active_notice_cnt = 0;
		}
	};

	auto& g_notice_mngr = c_notice_mngr::instance();
}
