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

#include "catapult/thread/Task.h"
#include "tests/TestHarness.h"

namespace catapult { namespace thread {

#define TEST_CLASS TaskTests

	using TimeSpan = utils::TimeSpan;

	// region CreateUniformDelayGenerator

	TEST(TEST_CLASS, UniformDelayGenerator_AlwaysReturnsRepeatDelay) {
		// Arrange:
		auto delayGenerator = CreateUniformDelayGenerator(TimeSpan::FromSeconds(3));

		// Act:
		for (auto i = 0u; i < 100; ++i) {
			auto delay = delayGenerator();

			// Assert:
			EXPECT_EQ(TimeSpan::FromSeconds(3), delay) << "at " << i;
		}
	}

	// endregion

	// region CreateIncreasingDelayGenerator

	TEST(TEST_CLASS, IncreasingDelayGenerator_RequiresMinDelayToBeLessThanMaxDelay) {
		EXPECT_THROW(CreateIncreasingDelayGenerator(TimeSpan::FromSeconds(3), 7, TimeSpan::FromSeconds(3), 5), catapult_invalid_argument);
		EXPECT_THROW(CreateIncreasingDelayGenerator(TimeSpan::FromSeconds(4), 7, TimeSpan::FromSeconds(3), 5), catapult_invalid_argument);
	}

	TEST(TEST_CLASS, IncreasingDelayGenerator_ReturnsMinDelayThroughoutPhaseOne) {
		// Arrange:
		auto delayGenerator = CreateIncreasingDelayGenerator(TimeSpan::FromSeconds(1), 7, TimeSpan::FromSeconds(3), 6);

		// Act:
		for (auto i = 0u; i < 7; ++i) {
			auto delay = delayGenerator();

			// Assert:
			EXPECT_EQ(TimeSpan::FromSeconds(1), delay) << "at " << i;
		}
	}

	TEST(TEST_CLASS, IncreasingDelayGenerator_ReturnsLinearlyIncreasingDelaysThroughoutPhaseTwo) {
		// Arrange:
		auto delayGenerator = CreateIncreasingDelayGenerator(TimeSpan::FromSeconds(1), 7, TimeSpan::FromSeconds(3), 6);

		// - skip to phase two
		for (auto i = 0u; i < 7; ++i)
			delayGenerator();

		// Act:
		std::vector<TimeSpan> delays;
		for (auto i = 0u; i < 5; ++i)
			delays.push_back(delayGenerator());

		// Assert:
		std::vector<TimeSpan> expectedDelays{
			TimeSpan::FromMilliseconds(1000 + 333),
			TimeSpan::FromMilliseconds(1000 + 666),
			TimeSpan::FromMilliseconds(1000 + 1000),
			TimeSpan::FromMilliseconds(1000 + 1333),
			TimeSpan::FromMilliseconds(1000 + 1666)
		};
		EXPECT_EQ(expectedDelays, delays);
	}

	TEST(TEST_CLASS, IncreasingDelayGenerator_ReturnsMaxDelayThroughoutPhaseThree) {
		// Arrange:
		auto delayGenerator = CreateIncreasingDelayGenerator(TimeSpan::FromSeconds(1), 7, TimeSpan::FromSeconds(3), 6);

		// - skip to phase three
		for (auto i = 0u; i < 7 + 5; ++i)
			delayGenerator();

		// Act:
		for (auto i = 0u; i < 100; ++i) {
			auto delay = delayGenerator();

			// Assert:
			EXPECT_EQ(TimeSpan::FromSeconds(3), delay) << "at " << i;
		}
	}

	// endregion

	// region Task

	TEST(TEST_CLASS, CanCreateDefaultTask) {
		// Act:
		Task task;

		// Assert:
		EXPECT_TRUE(task.Name.empty());
		EXPECT_EQ(TimeSpan(), task.StartDelay);
		EXPECT_FALSE(!!task.NextDelay);

		EXPECT_FALSE(!!task.Callback);
	}

	TEST(TEST_CLASS, CanCreateNamedTask) {
		// Act:
		auto counter = 0u;
		auto task = CreateNamedTask("foo task", [&counter]() {
			++counter;
			return thread::make_ready_future(thread::TaskResult::Continue);
		});

		// Assert:
		EXPECT_EQ("foo task", task.Name);
		EXPECT_EQ(TimeSpan(), task.StartDelay);
		EXPECT_FALSE(!!task.NextDelay);

		// - execute the callback (use counter as a proxy that the callback is set correctly)
		ASSERT_TRUE(!!task.Callback);
		task.Callback();
		EXPECT_EQ(1u, counter);
	}

	// endregion
}}
