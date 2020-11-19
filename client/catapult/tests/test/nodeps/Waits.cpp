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

#include "Waits.h"
#include "tests/TestHarness.h"
#include <chrono>
#include <thread>

namespace catapult { namespace test {

	void Pause() {
		Sleep(50);
	}

	void Sleep(long numMills) {
		std::this_thread::sleep_for(std::chrono::milliseconds(numMills));
	}

	namespace {
		DeterministicTimeSpan GetCurrentTime() {
			auto time = std::chrono::system_clock::now().time_since_epoch();
			return std::chrono::duration_cast<std::chrono::milliseconds>(time);
		}
	}

	DeterministicTimeSpan RunDeterministicOperation(const action& operation) {
		constexpr size_t Max_Attempts = 100;
		size_t i = 0;
		DeterministicTimeSpan operationStart;
		DeterministicTimeSpan operationEnd;

		do {
			if (++i > Max_Attempts) {
				auto duration = std::chrono::duration_cast<std::chrono::microseconds>(operationStart - operationEnd);
				CATAPULT_THROW_RUNTIME_ERROR_1("RunDeterministicOperation timed out with duration (um)", duration.count());
			}

			operationStart = GetCurrentTime();
			operation();
			operationEnd = GetCurrentTime();
		} while (operationStart != operationEnd);

		return operationEnd;
	}

	void RunNonDeterministicTest(const char* description, const predicate<>& test) {
		RunNonDeterministicTest(description, [test](auto) { return test(); });
	}

	void RunNonDeterministicTest(const char* description, const predicate<size_t>& test) {
		RunNonDeterministicTest(description, GetMaxNonDeterministicTestRetries(), test);
	}

	void RunNonDeterministicTest(const char* description, size_t numRetries, const predicate<size_t>& test) {
		auto i = 0u;
		while (i < numRetries) {
			if (test(++i)) {
				CATAPULT_LOG(debug) << description << " test was deterministic for iteration " << i;
				return;
			}

			CATAPULT_LOG(warning) << description << " test was inconclusive, retrying ...";
		}

		auto message = std::string(description) + " test was inconclusive for all iterations";
		CATAPULT_THROW_RUNTIME_ERROR_1(message.c_str(), numRetries);
	}

	std::chrono::nanoseconds GetCurrentTimeNanoseconds() {
		auto time = std::chrono::steady_clock::now().time_since_epoch();
		return std::chrono::duration_cast<std::chrono::nanoseconds>(time);
	}

	uint32_t GetTimeUnitForIteration(size_t i) {
		return static_cast<uint32_t>(i * 5);
	}
}}
