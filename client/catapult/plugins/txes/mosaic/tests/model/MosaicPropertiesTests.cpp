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

#include "src/model/MosaicProperties.h"
#include "catapult/utils/Casting.h"
#include "tests/test/MosaicTestUtils.h"
#include "tests/test/nodeps/Equality.h"
#include "tests/TestHarness.h"

namespace catapult { namespace model {

#define TEST_CLASS MosaicPropertiesTests

	// region ctor

	TEST(TEST_CLASS, CanCreateDefaultMosaicProperties) {
		// Act:
		MosaicProperties properties;

		// Assert:
		EXPECT_EQ(MosaicFlags::None, properties.flags());
		EXPECT_EQ(0u, properties.divisibility());
		EXPECT_EQ(BlockDuration(), properties.duration());

		for (auto flag = 1u; flag < utils::to_underlying_type(MosaicFlags::All); flag <<= 1)
			EXPECT_FALSE(properties.is(static_cast<MosaicFlags>(flag))) << "flag " << flag;
	}

	TEST(TEST_CLASS, CanCreateMosaicPropertiesAroundValues) {
		// Act:
		MosaicProperties properties(MosaicFlags::Supply_Mutable | MosaicFlags::Restrictable, 5, BlockDuration(234));

		// Assert:
		EXPECT_EQ(MosaicFlags::Supply_Mutable | MosaicFlags::Restrictable, properties.flags());
		EXPECT_EQ(5u, properties.divisibility());
		EXPECT_EQ(BlockDuration(234), properties.duration());

		for (auto flag = 1u; flag < utils::to_underlying_type(MosaicFlags::All); flag <<= 1) {
			EXPECT_EQ(
					MosaicFlags::Transferable != static_cast<MosaicFlags>(flag),
					properties.is(static_cast<MosaicFlags>(flag))) << "flag " << flag;
		}
	}

	// endregion

	// region equality operators

	namespace {
		std::unordered_set<std::string> GetEqualTags() {
			return { "default", "copy" };
		}

		std::unordered_map<std::string, MosaicProperties> GenerateEqualityInstanceMap() {
			return {
				{ "default", test::CreateMosaicPropertiesFromValues(2, 7, 5) },
				{ "copy", test::CreateMosaicPropertiesFromValues(2, 7, 5) },

				{ "diff[0]", test::CreateMosaicPropertiesFromValues(1, 7, 5) },
				{ "diff[1]", test::CreateMosaicPropertiesFromValues(2, 9, 5) },
				{ "diff[2]", test::CreateMosaicPropertiesFromValues(2, 7, 6) },
				{ "reverse", test::CreateMosaicPropertiesFromValues(5, 7, 2) },
				{ "diff-all", test::CreateMosaicPropertiesFromValues(1, 8, 6) }
			};
		}
	}

	TEST(TEST_CLASS, OperatorEqualReturnsTrueOnlyForEqualValues) {
		test::AssertOperatorEqualReturnsTrueForEqualObjects("default", GenerateEqualityInstanceMap(), GetEqualTags());
	}

	TEST(TEST_CLASS, OperatorNotEqualReturnsTrueOnlyForUnequalValues) {
		test::AssertOperatorNotEqualReturnsTrueForUnequalObjects("default", GenerateEqualityInstanceMap(), GetEqualTags());
	}

	// endregion
}}
