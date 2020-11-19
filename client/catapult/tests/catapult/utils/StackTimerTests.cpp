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

#include "catapult/utils/StackTimer.h"
#include "tests/TestHarness.h"

namespace catapult { namespace utils {

#define TEST_CLASS StackTimerTests

	TEST(TEST_CLASS, ElapsedMillisIsInitiallyZero) {
		// Arrange:
		uint64_t elapsedMillis;
		test::RunDeterministicOperation([&elapsedMillis]() {
			StackTimer stackTimer;

			// Act:
			elapsedMillis = stackTimer.millis();
		});

		// Assert:
		EXPECT_EQ(0u, elapsedMillis);
	}

	TEST(TEST_CLASS, ElapsedMillisIncreasesOverTime) {
		// Arrange:
		StackTimer stackTimer;

		// Act:
		test::Sleep(5);
		auto elapsedMillis1 = stackTimer.millis();
		test::Sleep(10);
		auto elapsedMillis2 = stackTimer.millis();

		// Assert:
		EXPECT_LE(0u, elapsedMillis1);
		EXPECT_LE(elapsedMillis1, elapsedMillis2);
	}

	namespace {
		constexpr auto Sleep_Millis = 5u;
		constexpr auto Epsilon_Millis = 1u;

		bool IsWithinSleepEpsilonRange(uint64_t millis) {
			return Sleep_Millis - Epsilon_Millis <= millis && millis <= Sleep_Millis + Epsilon_Millis;
		}
	}

	TEST(TEST_CLASS, ElapsedMillisCanBeAccessedAtPointInTime) {
		// Arrange: non-deterministic due to sleep
		test::RunNonDeterministicTest("specific elapsed time", []() {
			StackTimer stackTimer;

			// - wait
			test::Sleep(Sleep_Millis);

			// Act:
			auto elapsedMillis = stackTimer.millis();
			if (!IsWithinSleepEpsilonRange(elapsedMillis)) {
				CATAPULT_LOG(debug) << "elapsedMillis (" << elapsedMillis << ") outside of expected range";
				return false;
			}

			// Assert:
			EXPECT_TRUE(IsWithinSleepEpsilonRange(elapsedMillis)) << "elapsedMillis: " << elapsedMillis;
			return true;
		});
	}
}}
