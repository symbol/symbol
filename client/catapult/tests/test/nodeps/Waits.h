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
#include "catapult/utils/Logging.h"
#include "catapult/exceptions.h"
#include "catapult/functions.h"
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
		constexpr size_t Default_Wait_Timeout = 5;

		/// Waits for the specified function (\a func) to return the desired value (\a desired)
		/// with a configurable timeout (\a timeoutSeconds).
		template<typename T1, typename T2>
		bool TryWaitFor(const supplier<T1>& func, T2 desired, size_t timeoutSeconds) {
			auto begin = std::chrono::high_resolution_clock::now();

			while (desired != func()) {
				auto current = std::chrono::high_resolution_clock::now();
				auto elapsedSeconds = std::chrono::duration_cast<std::chrono::seconds>(current - begin).count();
				if (static_cast<size_t>(elapsedSeconds) > timeoutSeconds)
					return false;

				Sleep(10);
			}

			return true;
		}

		template<typename T>
		supplier<T> MakeFunction(const supplier<T>& func) {
			return func;
		}

		template<typename T>
		supplier<T> MakeFunction(const std::atomic<T>& value) {
			return [&value]() { return value.load(); };
		}
	}

/// Waits for the specified atomic value or function (\a SUPPLIER) to return the desired value (\a DESIRED) for \a TIMEOUT_SECONDS seconds.
#define WAIT_FOR_VALUE_SECONDS(DESIRED, SUPPLIER, TIMEOUT_SECONDS) \
	do { \
		auto func = test::detail::MakeFunction(SUPPLIER); \
		if (!test::detail::TryWaitFor(func, DESIRED, TIMEOUT_SECONDS)) { \
			EXPECT_EQ(DESIRED, func()) << "timeout " << TIMEOUT_SECONDS; \
			CATAPULT_THROW_RUNTIME_ERROR_2("WAIT_FOR_VALUE_SECONDS timed out waiting (desired, actual)", DESIRED, func()); \
		} \
	} while (false)

/// Waits for the specified \a EXPRESSION to return the desired value (\a DESIRED) for \a TIMEOUT_SECONDS seconds.
#define WAIT_FOR_VALUE_EXPR_SECONDS(DESIRED, EXPRESSION, TIMEOUT_SECONDS) \
	WAIT_FOR_VALUE_SECONDS(DESIRED, supplier<decltype(EXPRESSION)>([&]() { return EXPRESSION; }), TIMEOUT_SECONDS)

/// Waits for the specified atomic value or function (\a SUPPLIER) to return the desired value (\a DESIRED).
#define WAIT_FOR_VALUE(DESIRED, SUPPLIER) WAIT_FOR_VALUE_SECONDS(DESIRED, SUPPLIER, test::detail::Default_Wait_Timeout)

/// Waits for the specified \a EXPRESSION to return the desired value (\a DESIRED).
#define WAIT_FOR_VALUE_EXPR(DESIRED, EXPRESSION) WAIT_FOR_VALUE_EXPR_SECONDS(DESIRED, EXPRESSION, test::detail::Default_Wait_Timeout)

/// Waits for the specified atomic value or function (\a SUPPLIER) to change to \c true.
#define WAIT_FOR(SUPPLIER) WAIT_FOR_VALUE(true, SUPPLIER)

/// Waits for the specified \a EXPRESSION to change to \c true.
#define WAIT_FOR_EXPR(EXPRESSION) WAIT_FOR_VALUE_EXPR(true, EXPRESSION)

/// Waits for the specified atomic value or function (\a SUPPLIER) to change to \c 1.
#define WAIT_FOR_ONE(SUPPLIER) WAIT_FOR_VALUE(1u, SUPPLIER)

/// Waits for the specified \a EXPRESSION to change to \c 1.
#define WAIT_FOR_ONE_EXPR(EXPRESSION) WAIT_FOR_VALUE_EXPR(1u, EXPRESSION)

/// Waits for the specified atomic value or function (\a SUPPLIER) to change to \c 0.
#define WAIT_FOR_ZERO(SUPPLIER) WAIT_FOR_VALUE(0u, SUPPLIER)

/// Waits for the specified \a EXPRESSION to change to \c 0.
#define WAIT_FOR_ZERO_EXPR(EXPRESSION) WAIT_FOR_VALUE_EXPR(0u, EXPRESSION)

	/// Waits for \a pObject to be the last remaining shared pointer to the underlying object
	/// described by \a name.
	template<typename T>
	void WaitForUnique(const std::shared_ptr<T>& pObject, const std::string& name) {
		struct UseCount {};

		try {
			WAIT_FOR_ONE_EXPR(static_cast<unsigned long>(pObject.use_count()));
		} catch (catapult_runtime_error& ex) {
			auto useCount = pObject.use_count();
			ex << exception_detail::Make<UseCount>::From(useCount);
			CATAPULT_LOG(fatal) << "WaitForUnique failed for '" << name << "' with " << useCount << " remaining references";
			throw;
		}
	}

	/// Elapsed time (in milliseconds) at which a deterministic operation was run.
	using DeterministicTimeSpan = std::chrono::duration<uint64_t, std::milli>;

	/// Runs a deterministic \a operation.
	/// \note The deterministic operation must complete within a millisecond and is guaranteed to have the same start and end time.
	DeterministicTimeSpan RunDeterministicOperation(const action& operation);

	/// Runs a non deterministic \a test with \a description.
	void RunNonDeterministicTest(const char* description, const predicate<>& test);

	/// Runs a non deterministic \a test with \a description.
	/// \note The 1-based iteration number is passed to the test function.
	void RunNonDeterministicTest(const char* description, const predicate<size_t>& test);

	/// Runs a non deterministic \a test with \a description for \a numRetries attempts.
	/// \note The 1-based iteration number is passed to the test function.
	void RunNonDeterministicTest(const char* description, size_t numRetries, const predicate<size_t>& test);

	/// Gets the current time in nanoseconds.
	std::chrono::nanoseconds GetCurrentTimeNanoseconds();

	/// Gets the time unit for iteration \a i.
	/// \note Time units increase linearly with iteration.
	uint32_t GetTimeUnitForIteration(size_t i);

	/// Expects \a expected (with name \a expectedName) and \a actual (with name \a actualName) to be equal.
	/// \note If \a expected and \a actual are not equal, \c false is returned.
	template<typename T1, typename T2>
	bool ExpectEqualOrRetry(const T1& expected, const T2& actual, const char* expectedName, const char* actualName) {
		if (expected != actual) {
			CATAPULT_LOG(debug)
					<< "value of " << actualName << ": " << actual
					<< ", expected " << expectedName << " == " << expected << ", retrying...";
			return false;
		}

		EXPECT_EQ(expected, actual);
		return true;
	}
}}
