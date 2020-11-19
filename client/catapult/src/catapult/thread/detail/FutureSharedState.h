/**
*** Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
*** Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
*** All rights reserved.
***
*** This file is part of Catapult.
***
*** Catapult is free software: you can redistribute it and/or modify
*** it under the terms of the GNU Lesser General Public License as published by
*** the Free Software Foundation, either version 3 of the License, or
*** (at your option) any later version.
***
*** Catapult is distributed in the hope that it will be useful,
*** but WITHOUT ANY WARRANTY; without even the implied warranty of
*** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*** GNU Lesser General Public License for more details.
***
*** You should have received a copy of the GNU Lesser General Public License
*** along with Catapult. If not, see <http://www.gnu.org/licenses/>.
**/

#pragma once
#include "catapult/utils/NonCopyable.h"
#include "catapult/functions.h"
#include <future>
#include <memory>
#include <mutex>

namespace catapult { namespace thread { namespace detail {

	/// Shared state that is shared between a promise and a future.
	template<typename T>
	class shared_state : utils::NonCopyable {
	private:
		using ContinuationFunc = consumer<const std::shared_ptr<shared_state<T>>>;

		enum class future_status {
			pending,
			completed_success,
			completed_error
		};

	public:
		/// Creates an incomplete shared state.
		shared_state() : m_status(future_status::pending)
		{}

	public:
		/// Returns \c true if this shared state has completed and get will not block.
		bool is_ready() const {
			return future_status::pending != m_status;
		}

		/// Gets the result of this shared state and blocks until the result is available.
		T get() {
			if (m_status == future_status::pending) {
				std::unique_lock<std::mutex> lock(m_mutex);
				m_condition.wait(lock, [&status = m_status]() { return status != future_status::pending; });
			}

			if (future_status::completed_error == m_status)
				std::rethrow_exception(m_pException);

			return std::move(m_value);
		}

	public:
		/// Sets the result of this shared state to \a value.
		void set_value(T&& value) {
			std::lock_guard<std::mutex> lock(m_mutex);
			assert_pending();
			m_value = std::move(value);
			signal(future_status::completed_success);
		}

		/// Sets the result of this shared state to \a pException.
		void set_exception(std::exception_ptr pException) {
			std::lock_guard<std::mutex> lock(m_mutex);
			assert_pending();
			m_pException = pException;
			signal(future_status::completed_error);
		}

		/// Configures \a continuation to run at the completion of this shared state.
		void set_continuation(const ContinuationFunc& continuation) {
			std::lock_guard<std::mutex> lock(m_mutex);
			if (m_continuation)
				throw std::logic_error("continuation already set");

			m_continuation = continuation;
			if (is_ready())
				invokeContinuation();
		}

	private:
		void assert_pending() const {
			if (is_ready())
				throw std::future_error(std::future_errc::promise_already_satisfied);
		}

		void signal(future_status status) {
			m_status = status;
			m_condition.notify_all();
			if (!m_continuation)
				return;

			invokeContinuation();
		}

		void invokeContinuation() {
			auto pStateCopy = std::make_shared<shared_state<T>>();
			pStateCopy->m_status = m_status.load();
			pStateCopy->m_value = std::move(m_value);
			pStateCopy->m_pException = m_pException;
			m_continuation(pStateCopy);
		}

	private:
		std::atomic<future_status> m_status;
		T m_value;
		std::exception_ptr m_pException;
		ContinuationFunc m_continuation;

		std::condition_variable m_condition;
		std::mutex m_mutex;
	};
}}}
