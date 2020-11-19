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

#include "timesync/src/filters/SynchronizationFilters.h"
#include "timesync/tests/test/TimeSynchronizationTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace timesync { namespace filters {

#define TEST_CLASS ReversedTimestampsFilterTests

	TEST(TEST_CLASS, FiltersOutSamplesWithReversedLocalTimestamps) {
		// Arrange:
		auto filter = CreateReversedTimestampsFilter();

		// Act + Assert:
		for (auto i = 0; i < 5; ++i)
			EXPECT_TRUE(filter(test::CreateSample(i + 1, i, 10, 20), NodeAge())) << i;
	}

	TEST(TEST_CLASS, FiltersOutSamplesWithReversedReceivedTimestamps) {
		// Arrange:
		auto filter = CreateReversedTimestampsFilter();

		// Act + Assert:
		// note that remote should supply receive timestamp smaller or equal to send timestamp
		for (auto i = 0; i < 5; ++i)
			EXPECT_TRUE(filter(test::CreateSample(10, 20, i, i + 1), NodeAge())) << i;
	}

	TEST(TEST_CLASS, DoesNotFilterOutSamplesWithCorrectlyOrderedTimestamps) {
		// Arrange:
		auto filter = CreateReversedTimestampsFilter();

		// Act + Assert:
		for(auto i = 0; i < 5; ++i) {
			EXPECT_FALSE(filter(test::CreateSample(i, i + 1, i, i), NodeAge())) << i;
			EXPECT_FALSE(filter(test::CreateSample(i, i, i + 1, i), NodeAge())) << i;
		}
	}
}}}
