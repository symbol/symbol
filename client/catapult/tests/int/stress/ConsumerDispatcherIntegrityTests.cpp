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

#include "catapult/disruptor/ConsumerDispatcher.h"
#include "catapult/utils/HexFormatter.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/test/nodeps/Logging.h"
#include "tests/TestHarness.h"
#include <atomic>
#include <thread>
#include <vector>

namespace catapult { namespace disruptor {

#define TEST_CLASS ConsumerDispatcherIntegrityTests

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

	NO_STRESS_TEST(TEST_CLASS, MultipleConsumersCanProcessAllPushedElements) {
		// Arrange:
		test::GlobalLogFilter testLogFilter(utils::LogLevel::Info);
		auto ranges = test::PrepareRanges(1);

		std::atomic<uint64_t> counter1(Difficulty::Min().unwrap());
		std::atomic<uint64_t> counter2(0);
		std::atomic<uint64_t> inspectorCounter(0);
		ConsumerDispatcherOptions options{ "ConsumerDispatcherIntegrityTests", Default_Disruptor_Size };
		options.ElementTraceInterval = Elements_Per_Inner_Iteration * 8;
		ConsumerDispatcher dispatcher(
				options,
				{
					[&counter1](auto&) {
						++counter1;
						return ConsumerResult::Continue();
					},
					[&counter2](auto&) {
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

			// if the dispatcher is more than half full, wait for the consumers to catch up
			WAIT_FOR_EXPR(Default_Disruptor_Size / 2 > dispatcher.numAddedElements() - inspectorCounter);
		}

		// Assert (counter1 is modified by first consumer, counter2 by the second):
		constexpr uint64_t Iteration_Count = Outer_Iteration * Elements_Per_Inner_Iteration;
		WAIT_FOR_VALUE(Iteration_Count, inspectorCounter);

		EXPECT_EQ(Difficulty::Min().unwrap() + Iteration_Count, counter1);
		EXPECT_EQ(Iteration_Count, counter2);
		EXPECT_EQ(Iteration_Count, inspectorCounter);
	}
}}
