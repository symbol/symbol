#pragma once
#include "catapult/exceptions.h"
#include "catapult/preprocessor.h"
#include <atomic>
#include <chrono>
#include <thread>

namespace catapult { namespace tools {

	/// Default timeout used by WaitFor.
	constexpr size_t Default_Wait_Timeout = 1000;

	/// Puts this thread to sleep for a duration of \a milliseconds.
	CATAPULT_INLINE
	void Sleep(size_t milliseconds = Default_Wait_Timeout) {
		std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
	}

	/// Waits for the specified function (\a func) to return the desired value (\a desired) with a configurable
	/// timeout (\a timeoutMilliseconds). Returns \c true if the desired value was hit, \c false otherwise.
	template<typename T>
	bool TryWaitFor(
			const std::function<T()>& func,
			T desired,
			size_t timeoutMilliseconds = Default_Wait_Timeout) {
		auto begin = std::chrono::high_resolution_clock::now();

		while (desired != func()) {
			auto current = std::chrono::high_resolution_clock::now();
			auto elapsedMilliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(current - begin).count();
			if (static_cast<size_t>(elapsedMilliseconds) > timeoutMilliseconds) {
				return false;
			}

			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}

		return true;
	}

	/// Waits for the specified atomic (\a value) to change to the desired value (\a desired) with a configurable
	/// timeout (\a timeoutMilliseconds). Throws if a timeout occurs.
	template<typename T>
	void WaitFor(const std::atomic<T>& value, T desired, size_t timeoutMilliseconds = Default_Wait_Timeout) {
		if (!TryWaitFor<T>([&value]() { return value.load(); }, desired, timeoutMilliseconds))
			CATAPULT_THROW_RUNTIME_ERROR_2("WaitFor timed out waiting (desired, actual)", desired, value.load());
	}

	/// Waits for the specified atomic (\a value) to change to the desired value (\a desired) with a configurable
	/// timeout (\a timeoutMilliseconds). Returns \c true if the desired value was hit, \c false otherwise.
	template<typename T>
	bool TryWaitFor(const std::atomic<T>& value, T desired, size_t timeoutMilliseconds = Default_Wait_Timeout) {
		bool result = TryWaitFor<T>([&value]() { return value.load(); }, desired, timeoutMilliseconds);
		if (!result)
			CATAPULT_LOG(debug) << "WaitFor timed out waiting (desired, actual) " << desired << ", " << value.load();

		return result;
	}
}}
