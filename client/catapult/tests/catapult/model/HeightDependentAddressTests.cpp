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

#include "catapult/model/HeightDependentAddress.h"
#include "tests/TestHarness.h"

namespace catapult { namespace model {

#define TEST_CLASS HeightDependentAddressTests

	TEST(TEST_CLASS, CanCreateEmptyAddress) {
		// Arrange:
		auto heightDependentAddress = HeightDependentAddress();

		// Act + Assert:
		for (auto height : std::initializer_list<Height::ValueType>{ 0, 1, 99, 100, 101, 140, 141, 142 })
			EXPECT_EQ(Address(), heightDependentAddress.get(Height(height))) << "height " << height;
	}

	TEST(TEST_CLASS, CanCreateAddressAroundSingleValue) {
		// Arrange:
		auto addresses = test::GenerateRandomDataVector<Address>(1);
		auto heightDependentAddress = HeightDependentAddress(addresses[0]);

		// Act + Assert:
		for (auto height : std::initializer_list<Height::ValueType>{ 0, 1, 99, 100, 101, 140, 141, 142 })
			EXPECT_EQ(addresses[0], heightDependentAddress.get(Height(height))) << "height " << height;
	}

	TEST(TEST_CLASS, CanCreateAddressAroundMultipleValues) {
		// Arrange:
		auto addresses = test::GenerateRandomDataVector<Address>(3);
		auto heightDependentAddress = HeightDependentAddress(addresses[0]);

		std::vector<bool> trySetResults(2);
		trySetResults[0] = heightDependentAddress.trySet(addresses[1], Height(100));
		trySetResults[1] = heightDependentAddress.trySet(addresses[2], Height(141));

		// Sanity:
		EXPECT_EQ(std::vector<bool>({ true, true }), trySetResults);

		// Act + Assert:
		for (auto height : std::initializer_list<Height::ValueType>{ 1, 99 })
			EXPECT_EQ(addresses[1], heightDependentAddress.get(Height(height))) << "height " << height;

		for (auto height : std::initializer_list<Height::ValueType>{ 100, 101, 140 })
			EXPECT_EQ(addresses[2], heightDependentAddress.get(Height(height))) << "height " << height;

		for (auto height : std::initializer_list<Height::ValueType>{ 0, 141, 142 })
			EXPECT_EQ(addresses[0], heightDependentAddress.get(Height(height))) << "height " << height;
	}

	TEST(TEST_CLASS, CanCreateAddressAroundMultipleValuesIgnoringZeroHeights) {
		// Arrange:
		auto addresses = test::GenerateRandomDataVector<Address>(5);
		auto heightDependentAddress = HeightDependentAddress(addresses[0]);

		std::vector<bool> trySetResults(4);
		trySetResults[0] = heightDependentAddress.trySet(addresses[1], Height(0));
		trySetResults[1] = heightDependentAddress.trySet(addresses[2], Height(100));
		trySetResults[2] = heightDependentAddress.trySet(addresses[3], Height(0));
		trySetResults[3] = heightDependentAddress.trySet(addresses[4], Height(141));

		// Sanity:
		EXPECT_EQ(std::vector<bool>({ false, true, false, true }), trySetResults);

		// Act + Assert:
		for (auto height : std::initializer_list<Height::ValueType>{ 1, 99 })
			EXPECT_EQ(addresses[2], heightDependentAddress.get(Height(height))) << "height " << height;

		for (auto height : std::initializer_list<Height::ValueType>{ 100, 101, 140 })
			EXPECT_EQ(addresses[4], heightDependentAddress.get(Height(height))) << "height " << height;

		for (auto height : std::initializer_list<Height::ValueType>{ 0, 141, 142 })
			EXPECT_EQ(addresses[0], heightDependentAddress.get(Height(height))) << "height " << height;
	}

	TEST(TEST_CLASS, CannotCreateAddressAroundMultipleValuesOutOfOrder) {
		// Arrange:
		auto addresses = test::GenerateRandomDataVector<Address>(3);
		auto heightDependentAddress = HeightDependentAddress(addresses[0]);
		auto trySetResult1 = heightDependentAddress.trySet(addresses[1], Height(141));

		// Sanity:
		EXPECT_TRUE(trySetResult1);

		// Act + Assert:
		EXPECT_THROW(heightDependentAddress.trySet(addresses[2], Height(100)), catapult_invalid_argument);
	}
}}
