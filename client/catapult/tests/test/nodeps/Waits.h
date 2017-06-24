#pragma once
#include "catapult/utils/Logging.h"
#include "catapult/exceptions.h"
#include "tests/TestHarness.h"
#include <atomic>
#include <chrono>
#include <functional>

namespace catapult { namespace test {

	/// Sleeps a thread for a period of time to allow other work to continue.
	void Pause();

	/// Sleeps a thread for the specified amount of time (\a numMillis).
	void Sleep(long numMills);

	namespace detail {
		/// Default timeout used by WaitFor.
		const size_t Default_Wait_Timeout = 5;

		/// Waits for the specified function (\a func) to return the desired value (\a desired)
		/// with a configurable timeout (\a timeoutSeconds) using \a wait.
		template<typename TFunc>
		bool TryWaitFor(
				TFunc func,
				decltype(func()) desired,
				size_t timeoutSeconds = Default_Wait_Timeout,
				const std::function<void (uint32_t)>& wait = Sleep) {
			auto begin = std::chrono::high_resolution_clock::now();

			while (desired != func()) {
				auto current = std::chrono::high_resolution_clock::now();
				auto elapsedSeconds = std::chrono::duration_cast<std::chrono::seconds>(current - begin).count();
				if (static_cast<size_t>(elapsedSeconds) > timeoutSeconds)
					return false;

				wait(1);
			}

			return true;
		}

		template<typename T>
		std::function<T ()> MakeFunction(const std::function<T ()>& func) {
			return func;
		}

		template<typename T>
		std::function<T ()> MakeFunction(const std::atomic<T>& value) {
			return [&value]() { return value.load(); };
		}
	}

/// Waits for the specified atomic value or function (\a SUPPLIER) to return the desired value (\a DESIRED).
#define WAIT_FOR_VALUE(SUPPLIER, DESIRED) \
	if (!test::detail::TryWaitFor(test::detail::MakeFunction(SUPPLIER), DESIRED)) { \
		EXPECT_EQ(DESIRED, test::detail::MakeFunction(SUPPLIER)()) << "timeout " << test::detail::Default_Wait_Timeout; \
		CATAPULT_THROW_RUNTIME_ERROR_1("WAIT_FOR_VALUE timed out waiting for desired", DESIRED); \
	}

/// Waits for the specified \a EXPRESSION to return the desired value (\a DESIRED).
#define WAIT_FOR_VALUE_EXPR(EXPRESSION, DESIRED) \
	if (!test::detail::TryWaitFor([&]() { return EXPRESSION; }, DESIRED)) { \
		EXPECT_EQ(DESIRED, EXPRESSION) << "timeout " << test::detail::Default_Wait_Timeout; \
		CATAPULT_THROW_RUNTIME_ERROR_1("WAIT_FOR_VALUE_EXPR timed out waiting for desired", DESIRED); \
	}

/// Waits for the specified atomic value or function (\a SUPPLIER) to change to \c true.
#define WAIT_FOR(SUPPLIER) WAIT_FOR_VALUE(SUPPLIER, true)

/// Waits for the specified \a EXPRESSION to change to \c true.
#define WAIT_FOR_EXPR(EXPRESSION) WAIT_FOR_VALUE_EXPR(EXPRESSION, true)

/// Waits for the specified atomic value or function (\a SUPPLIER) to change to \c 1.
#define WAIT_FOR_ONE(SUPPLIER) WAIT_FOR_VALUE(SUPPLIER, 1u)

/// Waits for the specified \a EXPRESSION to change to \c 1.
#define WAIT_FOR_ONE_EXPR(EXPRESSION) WAIT_FOR_VALUE_EXPR(EXPRESSION, 1u)

	/// Waits for \a pObject to be the last remaining shared pointer to the underlying object
	/// described by \a name.
	template<typename T>
	void WaitForUnique(const std::shared_ptr<T>& pObject, const std::string& name) {
		struct UseCount {};

		try {
			WAIT_FOR_EXPR(pObject.unique());
		} catch (catapult_runtime_error& ex) {
			auto useCount = pObject.use_count();
			ex << exception_detail::Make<UseCount>::From(useCount);
			CATAPULT_LOG(fatal) << "WaitForUnique failed for '" << name << "' with " << useCount << " remaining references";
			throw;
		}
	}

	/// The time (in milliseconds) at which a deterministic operation was run.
	using DeterministicTimeSpan = std::chrono::duration<uint64_t, std::milli>;

	/// Runs a deterministic \a operation.
	DeterministicTimeSpan RunDeterministicOperation(const std::function<void()>& operation);

	/// Runs a non deterministic \a test with \a description.
	void RunNonDeterministicTest(const char* description, const std::function<bool ()>& test);

	/// Runs a non deterministic \a test with \a description.
	/// \note The iteration number is passed to the test function.
	void RunNonDeterministicTest(const char* description, const std::function<bool (size_t)>& test);
}}
