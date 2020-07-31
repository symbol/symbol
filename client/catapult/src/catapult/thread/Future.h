/**
*** Copyright (c) 2016-present,
*** Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp. All rights reserved.
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
#include "detail/FutureSharedState.h"
#include "catapult/utils/NonCopyable.h"

namespace catapult { namespace thread {

	/// Provides a way to access the result of an asynchronous operation.
	template<typename T>
	class future : public utils::MoveOnly {
	public:
		/// Creates a default future.
		future() = default;

		/// Constructs a future around a shared state (\a pState).
		explicit future(const std::shared_ptr<detail::shared_state<T>>& pState) : m_pState(pState)
		{}

	public:
		/// Returns \c true if this future is valid.
		bool valid() {
			return !!m_pState;
		}

		/// Returns \c true if this future has completed and get will not block.
		bool is_ready() const {
			return m_pState->is_ready();
		}

		/// Gets the result of this future and blocks until the result is available.
		T get() {
			return m_pState->get();
		}

		/// Configures \a continuation to run at the completion of this future.
		template<
				typename TContinuation,
				typename TResultType = std::invoke_result_t<TContinuation, future<T>&&>
		>
		auto then(TContinuation continuation) {
			if constexpr (!std::is_same_v<TResultType, void>) {
				auto pResultState = std::make_shared<detail::shared_state<TResultType>>();
				m_pState->set_continuation([pResultState, continuation](const auto& pState) {
					try {
						pResultState->set_value(continuation(future<T>(pState)));
					} catch (...) {
						pResultState->set_exception(std::current_exception());
					}
				});

				return future<TResultType>(pResultState);
			} else {
				return then([continuation](auto&& nextFuture) {
					continuation(std::move(nextFuture));
					return true;
				});
			}
		}

	private:
		std::shared_ptr<detail::shared_state<T>> m_pState;
	};

	/// Stores the result of an asynchronous operation.
	template<typename T>
	class promise : public utils::MoveOnly {
	public:
		/// Constructs a promise.
		promise()
				: m_pState(std::make_shared<detail::shared_state<T>>())
				, m_pIsFutureCreated(std::make_unique<std::atomic_flag>()) {
			m_pIsFutureCreated->clear(std::memory_order_release);
		}

	public:
		/// Returns \c true if this promise is valid.
		bool valid() {
			return !!m_pState;
		}

		/// Gets a future associated with this promise.
		future<T> get_future() {
			if (m_pIsFutureCreated->test_and_set(std::memory_order_acquire))
				throw std::future_error(std::future_errc::future_already_retrieved);

			return future<T>(m_pState);
		}

	public:
		/// Sets the result of this promise to \a value.
		void set_value(T&& value) {
			m_pState->set_value(std::move(value));
		}

		/// Sets the result of this promise to \a pException.
		void set_exception(std::exception_ptr pException) {
			m_pState->set_exception(pException);
		}

	private:
		std::shared_ptr<detail::shared_state<T>> m_pState;
		std::unique_ptr<std::atomic_flag> m_pIsFutureCreated;
	};

	/// Produces a future that is ready immediately and holds the given \a value.
	template<typename T>
	future<T> make_ready_future(T&& value) {
		auto pState = std::make_shared<detail::shared_state<T>>();
		pState->set_value(std::move(value));
		return future<T>(pState);
	}

	/// Produces a future that is ready immediately and holds the given exception (\a ex).
	template<typename T, typename E>
	future<T> make_exceptional_future(E ex) {
		auto pState = std::make_shared<detail::shared_state<T>>();
		pState->set_exception(std::make_exception_ptr(ex));
		return future<T>(pState);
	}
}}
