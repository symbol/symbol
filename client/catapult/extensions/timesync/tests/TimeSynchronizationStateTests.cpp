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

#include "timesync/src/TimeSynchronizationState.h"
#include "catapult/utils/NetworkTime.h"
#include "tests/test/nodeps/Waits.h"
#include "tests/TestHarness.h"

namespace catapult { namespace timesync {

#define TEST_CLASS TimeSynchronizationStateTests

	namespace {
		constexpr auto Default_Epoch_Adjustment = utils::TimeSpan::FromMilliseconds(11223344556677);
		constexpr uint64_t Default_Threshold(123);
	}

	// region ctor

	TEST(TEST_CLASS, CanDefaultConstructState) {
		// Act:
		TimeSynchronizationState state(Default_Epoch_Adjustment, Default_Threshold);

		// Assert:
		EXPECT_EQ(TimeOffset(), state.offset());
		EXPECT_EQ(0u, state.absoluteOffset());
		EXPECT_EQ(TimeOffsetDirection::Positive, state.offsetDirection());
		EXPECT_EQ(NodeAge(), state.nodeAge());
	}

	// endregion

	// region update

	namespace {
		void AssertUpdateDoesNotChangeState(uint64_t threshold, int64_t offset) {
			// Arrange:
			TimeSynchronizationState state(Default_Epoch_Adjustment, threshold);

			// Act:
			state.update(TimeOffset(offset));

			// Assert:
			EXPECT_EQ(TimeOffset(), state.offset());
			EXPECT_EQ(0u, state.absoluteOffset());
			EXPECT_EQ(TimeOffsetDirection::Positive, state.offsetDirection());
			EXPECT_EQ(NodeAge(1), state.nodeAge());
		}
	}

	TEST(TEST_CLASS, UpdateIncreasesNodeAge) {
		// Arrange:
		TimeSynchronizationState state(Default_Epoch_Adjustment, Default_Threshold);

		// Act:
		state.update(TimeOffset());

		// Assert:
		EXPECT_EQ(TimeOffset(), state.offset());
		EXPECT_EQ(0u, state.absoluteOffset());
		EXPECT_EQ(TimeOffsetDirection::Positive, state.offsetDirection());
		EXPECT_EQ(NodeAge(1), state.nodeAge());
	}

	TEST(TEST_CLASS, UpdateDoesNotChangeOffsetWhenSuppliedOffsetIsSmallerThanThreshold_Positive) {
		AssertUpdateDoesNotChangeState(Default_Threshold, 50);
	}

	TEST(TEST_CLASS, UpdateDoesNotChangeOffsetWhenSuppliedOffsetIsSmallerThanThreshold_Negative) {
		AssertUpdateDoesNotChangeState(Default_Threshold, -50);
	}

	TEST(TEST_CLASS, UpdateCanChangeOffsetInPositiveDirection) {
		// Arrange:
		TimeSynchronizationState state(Default_Epoch_Adjustment, Default_Threshold);

		// Act:
		state.update(TimeOffset(150));

		// Assert:
		EXPECT_EQ(TimeOffset(150), state.offset());
		EXPECT_EQ(150u, state.absoluteOffset());
		EXPECT_EQ(TimeOffsetDirection::Positive, state.offsetDirection());
		EXPECT_EQ(NodeAge(1), state.nodeAge());
	}

	TEST(TEST_CLASS, UpdateCanChangeOffsetInNegativeDirection) {
		// Arrange:
		TimeSynchronizationState state(Default_Epoch_Adjustment, Default_Threshold);

		// Act:
		state.update(TimeOffset(-150));

		// Assert:
		EXPECT_EQ(TimeOffset(-150), state.offset());
		EXPECT_EQ(150u, state.absoluteOffset());
		EXPECT_EQ(TimeOffsetDirection::Negative, state.offsetDirection());
		EXPECT_EQ(NodeAge(1), state.nodeAge());
	}

	TEST(TEST_CLASS, UpdateRespectsExistingState) {
		// Arrange:
		TimeSynchronizationState state(Default_Epoch_Adjustment, Default_Threshold);

		// Act: threshold is 125, -150 + 350 - 250 = -50
		for (auto rawOffset : { -150, -50, 100, 350, -250})
			state.update(TimeOffset(rawOffset));

		// Assert:
		EXPECT_EQ(TimeOffset(-50), state.offset());
		EXPECT_EQ(50u, state.absoluteOffset());
		EXPECT_EQ(TimeOffsetDirection::Negative, state.offsetDirection());
		EXPECT_EQ(NodeAge(5), state.nodeAge());
	}

	// endregion

	// region networkTime

	TEST(TEST_CLASS, NetworkTimeUsesOffset) {
		// Arrange:
		TimeSynchronizationState state(Default_Epoch_Adjustment, Default_Threshold);
		state.update(TimeOffset(234));
		Timestamp timestamp;
		Timestamp networkTime;

		// Act:
		test::RunDeterministicOperation([&state, &timestamp, &networkTime]() {
			timestamp = utils::NetworkTime(Default_Epoch_Adjustment).now();
			networkTime = state.networkTime();
		});

		// Assert:
		EXPECT_EQ(timestamp + Timestamp(234), networkTime);
	}

	// endregion
}}
