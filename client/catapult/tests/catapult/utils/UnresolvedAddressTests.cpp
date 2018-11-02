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

#include "catapult/utils/UnresolvedAddress.h"
#include "tests/test/nodeps/Equality.h"
#include "tests/TestHarness.h"

namespace catapult { namespace utils {

#define TEST_CLASS UnresolvedAddressTests

	// region equality operators

	namespace {
		std::unordered_map<std::string, UnresolvedAddressByte> GenerateEqualityInstanceMap() {
			return { { "default", { 111 } }, { "222", { 222 } }, { "111", { 111 } } };
		}
	}

	TEST(TEST_CLASS, UnresolvedAddressByte_OperatorEqualReturnsTrueOnlyForEqualValues) {
		// Assert:
		test::AssertOperatorEqualReturnsTrueForEqualObjects("111", GenerateEqualityInstanceMap(), { "default", "111" });
	}

	TEST(TEST_CLASS, UnresolvedAddressByte_OperatorNotEqualReturnsTrueOnlyForUnequalValues) {
		// Assert:
		test::AssertOperatorNotEqualReturnsTrueForUnequalObjects("111", GenerateEqualityInstanceMap(), { "default", "111" });
	}

	// endregion

	TEST(TEST_CLASS, AddressAndUnresolvedAddressHaveSameSize) {
		// Assert:
		EXPECT_EQ(sizeof(Address), sizeof(UnresolvedAddress));
	}
}}
