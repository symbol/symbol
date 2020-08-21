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

#include "finalization/src/model/FinalizationStatistics.h"
#include "tests/test/nodeps/Alignment.h"
#include "tests/TestHarness.h"

namespace catapult { namespace model {

#define TEST_CLASS FinalizationStatisticsTests

	// region FinalizationStatistics (size + alignment)

#define STATISTICS_FIELDS FIELD(Point) FIELD(Height) FIELD(Hash)

	TEST(TEST_CLASS, FinalizationStatisticsHasExpectedSize) {
		// Arrange:
		auto expectedSize = 0u;

#define FIELD(X) expectedSize += SizeOf32<decltype(FinalizationStatistics::X)>();
		STATISTICS_FIELDS
#undef FIELD

		// Assert:
		EXPECT_EQ(expectedSize, sizeof(FinalizationStatistics));
		EXPECT_EQ(48u, sizeof(FinalizationStatistics));
	}

	TEST(TEST_CLASS, FinalizationStatisticsHasProperAlignment) {
#define FIELD(X) EXPECT_ALIGNED(FinalizationStatistics, X);
		STATISTICS_FIELDS
#undef FIELD

		EXPECT_EQ(0u, sizeof(FinalizationStatistics) % 8);
	}

#undef STATISTICS_FIELDS

	// endregion
}}
