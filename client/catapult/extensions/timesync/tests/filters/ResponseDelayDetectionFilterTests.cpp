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
#include "timesync/src/filters/filter_constants.h"
#include "timesync/tests/test/TimeSynchronizationTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace timesync { namespace filters {

#define TEST_CLASS ResponseDelayDetectionFilterTests

	TEST(TEST_CLASS, FiltersOutSamplesWithIntolerableDuration) {
		// Arrange:
		auto filter = CreateResponseDelayDetectionFilter();

		// Act + Assert:
		for (int64_t offset : { 1, 2, 10, 1000 })
			EXPECT_TRUE(filter(test::CreateTimeSyncSampleWithDuration(Tolerated_Duration_Maximum + offset), NodeAge()));
	}

	TEST(TEST_CLASS, DoesNotFilterOutSamplesWithTolerableDuration) {
		// Arrange:
		auto filter = CreateResponseDelayDetectionFilter();

		// Act + Assert:
		for (int64_t offset : { 0, -1, -2, -10, -1000 })
			EXPECT_FALSE(filter(test::CreateTimeSyncSampleWithDuration(Tolerated_Duration_Maximum + offset), NodeAge()));
	}
}}}
