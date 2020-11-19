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

#include "catapult/utils/DiagnosticCounter.h"
#include "tests/TestHarness.h"

namespace catapult { namespace utils {

#define TEST_CLASS DiagnosticCounterTests

	TEST(TEST_CLASS, CanCreateCounter) {
		// Act:
		DiagnosticCounter counter(DiagnosticCounterId("CAT"), []() { return 123; });

		// Assert:
		EXPECT_EQ("CAT", counter.id().name());
		EXPECT_EQ(123u, counter.value());
	}

	TEST(TEST_CLASS, CounterValueAccessesSupplierForLatestValue) {
		// Arrange:
		auto i = 0;
		DiagnosticCounter counter(DiagnosticCounterId(), [&i]() {
			++i;
			return i * i;
		});

		// Act:
		auto value1 = counter.value();
		auto value2 = counter.value();
		auto value3 = counter.value();

		// Assert:
		EXPECT_EQ(1u, value1);
		EXPECT_EQ(4u, value2);
		EXPECT_EQ(9u, value3);
	}
}}
