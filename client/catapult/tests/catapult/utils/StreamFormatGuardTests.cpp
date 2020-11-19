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

#include "catapult/utils/StreamFormatGuard.h"
#include "tests/TestHarness.h"

namespace catapult { namespace utils {

#define TEST_CLASS StreamFormatGuardTests

	TEST(TEST_CLASS, ConstructorSetsCustomFormattingSettings) {
		// Arrange:
		std::ostringstream out;
		out.flags(std::ios::hex | std::ios::fixed);
		out.fill('~');

		// Act:
		StreamFormatGuard guard(out, std::ios::oct | std::ios::right, 'X');

		// Assert:
		EXPECT_EQ(std::ios::oct | std::ios::right, out.flags());
		EXPECT_EQ('X', out.fill());
	}

	TEST(TEST_CLASS, DestructorRestoresOriginalFormattingSettings) {
		// Arrange:
		std::ostringstream out;
		out.flags(std::ios::hex | std::ios::fixed);
		out.fill('~');

		// Act:
		{
			StreamFormatGuard guard(out, std::ios::oct | std::ios::right, 'X');
		}

		// Assert:
		EXPECT_EQ(std::ios::hex | std::ios::fixed, out.flags());
		EXPECT_EQ('~', out.fill());
	}
}}
