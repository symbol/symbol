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
#include "tests/test/nodeps/Comparison.h"
#include "tests/test/nodeps/Equality.h"
#include "tests/TestHarness.h"

namespace catapult { namespace utils {

#define TEST_CLASS UnresolvedAddressTests

	// region UnresolvedAddressByte

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

	TEST(TEST_CLASS, UnresolvedAddressByte_OperatorLessThanReturnsTrueForSmallerValuesAndFalseOtherwise) {
		// Arrange:
		std::vector<UnresolvedAddressByte> bytes{ { 11 }, { 12 }, { 15 }, { 99 } };

		// Assert:
		test::AssertLessThanOperatorForEqualValues<UnresolvedAddressByte>({ 11 }, { 11 });
		test::AssertOperatorBehaviorForIncreasingValues(bytes, std::less<>(), [](const auto& byte) {
			std::ostringstream out;
			out << byte.Byte;
			return out.str();
		});
	}

	// endregion

	// region UnresolvedAddress

	TEST(TEST_CLASS, AddressAndUnresolvedAddressHaveSameSize) {
		// Assert:
		EXPECT_EQ(sizeof(Address), sizeof(UnresolvedAddress));
	}

	// endregion

	// region UnresolvedAddressHasher

	namespace {
		UnresolvedAddress GenerateRandomUnresolvedAddress() {
			UnresolvedAddress address;
			test::FillWithRandomData({ reinterpret_cast<uint8_t*>(address.data()), address.size() });
			return address;
		}
	}

	TEST(TEST_CLASS, UnresolvedAddressHasher_SameObjectReturnsSameHash) {
		// Arrange:
		UnresolvedAddressHasher hasher;
		auto address1 = GenerateRandomUnresolvedAddress();

		// Act:
		auto result1 = hasher(address1);
		auto result2 = hasher(address1);

		// Assert:
		EXPECT_EQ(result1, result2);
	}

	TEST(TEST_CLASS, UnresolvedAddressHasher_EqualObjectsReturnSameHash) {
		// Arrange:
		UnresolvedAddressHasher hasher;
		auto address1 = GenerateRandomUnresolvedAddress();
		auto address2 = address1;

		// Act:
		auto result1 = hasher(address1);
		auto result2 = hasher(address2);

		// Assert:
		EXPECT_EQ(result1, result2);
	}

	TEST(TEST_CLASS, UnresolvedAddressHasher_DifferentObjectsReturnDifferentHashes) {
		// Arrange:
		UnresolvedAddressHasher hasher;
		auto address1 = GenerateRandomUnresolvedAddress();
		UnresolvedAddress address2;
		std::transform(address1.cbegin(), address1.cend(), address2.begin(), [](auto byte) {
			return UnresolvedAddressByte{ static_cast<uint8_t>(byte.Byte ^ 0xFF) };
		});

		// Act:
		auto result1 = hasher(address1);
		auto result2 = hasher(address2);

		// Assert:
		EXPECT_NE(result1, result2);
	}

	// endregion
}}
