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

	DeterministicTimeSpan RunDeterministicOperation(const std::function<void()>& operation) {
		constexpr size_t Max_Attempts = 100;
		size_t i = 0;
		DeterministicTimeSpan operationStart;
		DeterministicTimeSpan operationEnd;

		do {
			if (++i > Max_Attempts) {
				auto duration = std::chrono::duration_cast<std::chrono::microseconds>(operationStart - operationEnd);
				CATAPULT_THROW_RUNTIME_ERROR_1(
						"RunDeterministicOperation timed out with duration (um)",
						duration.count());
			}

			operationStart = GetCurrentTime();
			operation();
			operationEnd = GetCurrentTime();
		} while (operationStart != operationEnd);

		return operationEnd;
	}

	void RunNonDeterministicTest(const char* description, const std::function<bool ()>& test) {
		RunNonDeterministicTest(description, [test](auto) { return test(); });
	}

	void RunNonDeterministicTest(const char* description, const std::function<bool (size_t)>& test) {
		constexpr auto Num_Retries = test::Max_Non_Deterministic_Test_Retries;
		auto i = 0u;
		while (i < Num_Retries) {
			if (test(++i)) {
				CATAPULT_LOG(debug) << description << " test was deterministic for iteration " << i;
				return;
			}

			CATAPULT_LOG(warning) << description << " test was inconclusive, retrying ...";
		}

		auto message = std::string(description) + " test was inconclusive for all iterations";
		CATAPULT_THROW_RUNTIME_ERROR_1(message.c_str(), Num_Retries);
	}
}}
