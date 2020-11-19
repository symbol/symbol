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
#include "Future.h"
#include "catapult/exceptions.h"
#include <atomic>
#include <vector>

namespace catapult { namespace thread {

	/// Gets a future that is signaled when all futures in \a allFutures complete.
	template<typename T>
	future<std::vector<future<T>>> when_all(std::vector<future<T>>&& allFutures) {
// workaround gcc bug by explicitly specifying inner struct visibility when inner struct contains lambda
// https://www.mail-archive.com/gcc-bugs@gcc.gnu.org/msg534746.html
#ifdef __GNUC__
#define INNER_STRUCT_VISIBILILTY __attribute__ ((visibility ("hidden")))
#else
#define INNER_STRUCT_VISIBILILTY
#endif

		using FutureType = future<T>;
		using JointPromiseType = promise<std::vector<FutureType>>;

		struct INNER_STRUCT_VISIBILILTY ContinuationContext : public std::enable_shared_from_this<ContinuationContext> {
		public:
			explicit ContinuationContext(size_t numFutures) : m_futures(numFutures), m_counter(0)
			{}

		public:
			auto future() {
				return m_jointPromise.get_future();
			}

			void setContinuation(FutureType& future, size_t index) {
				auto pThis = this->shared_from_this();
				future.then([pThis, index](auto&& continuationFuture) {
					auto& futures = pThis->m_futures;
					futures[index] = std::move(continuationFuture);
					if (futures.size() != ++pThis->m_counter)
						return;

					pThis->m_jointPromise.set_value(std::move(futures));
				});
			}

		private:
			std::vector<FutureType> m_futures;
			JointPromiseType m_jointPromise;
			std::atomic<size_t> m_counter;
		};

		if (allFutures.empty())
			CATAPULT_THROW_INVALID_ARGUMENT("when_all cannot join zero futures");

		auto i = 0u;
		auto pContext = std::make_shared<ContinuationContext>(allFutures.size());
		for (auto& future : allFutures)
			pContext->setContinuation(future, i++);

		return pContext->future();

#undef INNER_STRUCT_VISIBILILTY
	}

	/// Gets a future that is signaled when both \a future1 and \a future2 complete.
	template<typename T>
	future<std::vector<future<T>>> when_all(future<T>&& future1, future<T>&& future2) {
		std::vector<future<T>> futures;
		futures.push_back(std::move(future1));
		futures.push_back(std::move(future2));
		return when_all(std::move(futures));
	}

	/// Composes a future (\a startFuture) with a second future returned by a function (\a createNextFuture) that
	/// is passed the original future upon its completion.
	///
	/// \note The future returned by this function will be the same type as the one returned by \a createNextFuture.
	template<
			typename TSeed,
			typename TCreateNextFuture,
			typename TResultFuture = std::invoke_result_t<TCreateNextFuture, future<TSeed>&&>,
			typename TResultType = decltype(TResultFuture().get())>
	auto compose(future<TSeed>&& startFuture, TCreateNextFuture createNextFuture) {
		auto pComposePromise = std::make_shared<promise<TResultType>>();
		startFuture.then([createNextFuture, pComposePromise](auto&& completedFirstFuture) {
			try {
				auto secondFuture = createNextFuture(std::move(completedFirstFuture));
				secondFuture.then([pComposePromise](auto&& completedSecondFuture) {
					try {
						pComposePromise->set_value(completedSecondFuture.get());
					} catch (...) {
						pComposePromise->set_exception(std::current_exception());
					}
				});
			} catch (...) {
				pComposePromise->set_exception(std::current_exception());
			}
		});

		return pComposePromise->get_future();
	}

	/// Evaluates all \a futures and returns the results of all non-exceptional ones.
	/// \note Exceptional results are ignored and filtered out.
	template<typename T>
	std::vector<T> get_all_ignore_exceptional(std::vector<future<T>>&& futures) {
		std::vector<T> results;
		results.reserve(futures.size());

		for (auto& future : futures) {
			try {
				results.emplace_back(future.get());
			} catch (...) {
				// suppress
			}
		}

		return results;
	}

	/// Evaluates all \a futures and returns the results of all.
	/// \note This function will throw if at least one future has an exceptional result.
	template<typename T>
	std::vector<T> get_all(std::vector<future<T>>&& futures) {
		std::vector<T> results;
		results.reserve(futures.size());

		for (auto& future : futures)
			results.emplace_back(future.get());

		return results;
	}
}}
