#pragma once
#include "Future.h"
#include "catapult/exceptions.h"
#include <atomic>
#include <vector>

namespace catapult { namespace thread {

	/// Returns a future that is signaled when all futures in \a allFutures complete.
	template<typename T>
	future<std::vector<future<T>>> when_all(std::vector<future<T>>&& allFutures) {
		using FutureType = future<T>;
		using JointPromiseType = promise<std::vector<FutureType>>;

		struct ContinuationContext : public std::enable_shared_from_this<ContinuationContext> {
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
	}

	/// Returns a future that is signaled when both \a future1 and \a future2 complete.
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
			typename TResultFuture = typename std::result_of<TCreateNextFuture(future<TSeed>&&)>::type,
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
		for (auto& future : futures) {
			try {
				results.push_back(future.get());
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
		for (auto& future : futures)
			results.push_back(future.get());

		return results;
	}
}}
