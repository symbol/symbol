#include "catapult/disruptor/ConsumerDispatcher.h"
#include "catapult/utils/HexFormatter.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/test/nodeps/Logging.h"
#include "tests/TestHarness.h"
#include <atomic>
#include <thread>
#include <vector>

namespace catapult { namespace disruptor {

	namespace {
#ifdef STRESS
		constexpr size_t Adjustment_Divisor = 1u;
#else
		constexpr size_t Adjustment_Divisor = 8u;
#endif

		constexpr uint64_t Elements_Per_Inner_Iteration = 1024 / Adjustment_Divisor;
		constexpr uint64_t Outer_Iteration = 256u * 15 / Adjustment_Divisor;
		constexpr size_t Default_Disruptor_Size = 16 * 1024 / Adjustment_Divisor;
	}

	NO_STRESS_TEST(ConsumerDispatcherIntegrityTests, MultipleConsumersCanProcessAllPushedElements) {
		// Arrange:
		test::GlobalLogFilter testLogFilter(utils::LogLevel::Info);
		auto ranges = test::PrepareRanges(1);

		std::atomic<uint64_t> counter1(Difficulty::Min().unwrap());
		std::atomic<uint64_t> counter2(0);
		std::atomic<uint64_t> inspectorCounter(0);
		ConsumerDispatcher dispatcher(
				{ "ConsumerDispatcherIntegrityTests", Default_Disruptor_Size },
				{
					[&counter1](ConsumerInput&) -> ConsumerResult {
						++counter1;
						return ConsumerResult::Continue();
					},
					[&counter2](ConsumerInput&) -> ConsumerResult {
						++counter2;
						return ConsumerResult::Continue();
					}
				},
				[&inspectorCounter](const auto&, const auto&) { ++inspectorCounter; });

		// Act:
		for (auto j = 1u; j <= Outer_Iteration; ++j) {
			for (auto i = 0u; i < Elements_Per_Inner_Iteration; ++i) {
				auto range = model::BlockRange::CopyRange(ranges[0]);
				dispatcher.processElement(ConsumerInput(std::move(range)));
			}

			// if producer is further than "half of disruptor", let it sleep for a bit
			// and let the consumer keep up
			if (j * Elements_Per_Inner_Iteration - counter2 > (Default_Disruptor_Size / 2))
				std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}

		// Assert (counter1 is modified by first consumer, counter2 by the second):
		constexpr uint64_t Iteration_Count = Outer_Iteration * Elements_Per_Inner_Iteration;
		WAIT_FOR_VALUE(inspectorCounter, Iteration_Count);

		EXPECT_EQ(Difficulty::Min().unwrap() + Iteration_Count, counter1);
		EXPECT_EQ(Iteration_Count, counter2);
		EXPECT_EQ(Iteration_Count, inspectorCounter);
	}
}}
