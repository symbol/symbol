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

#include "catapult/utils/ImmutableValue.h"
#include "tests/TestHarness.h"

namespace catapult { namespace utils {

#define TEST_CLASS ImmutableValueTests

	TEST(TEST_CLASS, CanCreateAsConstexpr) {
		// Act:
		constexpr ImmutableValue<uint32_t> Const_Value(78);

		// Assert:
		EXPECT_EQ(78u, Const_Value);
	}

	TEST(TEST_CLASS, CanCreateAroundMutableValue) {
		// Act:
		ImmutableValue<uint32_t> value(78);

		// Assert:
		EXPECT_EQ(78u, value);
	}

	TEST(TEST_CLASS, CanMoveConstruct) {
		// Act:
		ImmutableValue<uint32_t> value(78);
		ImmutableValue<uint32_t> value2(std::move(value));

		// Assert:
		EXPECT_EQ(78u, value2);
	}

	TEST(TEST_CLASS, CanMoveAssign) {
		// Act:
		ImmutableValue<uint32_t> value(78);
		ImmutableValue<uint32_t> value2(12);
		const auto& assignResult = (value2 = std::move(value));

		// Assert:
		EXPECT_EQ(78u, value2);
		EXPECT_EQ(&value2, &assignResult);
	}

	TEST(TEST_CLASS, CanCastToMutableValue) {
		// Act:
		auto rawValue = static_cast<uint32_t>(78);

		// Assert:
		EXPECT_EQ(78u, rawValue);
	}
}}
