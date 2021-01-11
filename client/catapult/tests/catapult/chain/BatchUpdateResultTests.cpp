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

#include "catapult/chain/BatchUpdateResult.h"
#include "tests/test/nodeps/Equality.h"
#include "tests/TestHarness.h"

namespace catapult { namespace chain {

#define TEST_CLASS BatchUpdateResultTests

	// region creation

	TEST(TEST_CLASS, CanCreateDefaultResult) {
		// Act:
		auto result = BatchUpdateResult();

		// Assert:
		EXPECT_EQ(0u, result.SuccessCount);
		EXPECT_EQ(0u, result.NeutralCount);
		EXPECT_EQ(0u, result.FailureCount);
	}

	TEST(TEST_CLASS, CanCreateResultWithInitialValues) {
		// Act:
		auto result = BatchUpdateResult(77, 12, 98);

		// Assert:
		EXPECT_EQ(77u, result.SuccessCount);
		EXPECT_EQ(12u, result.NeutralCount);
		EXPECT_EQ(98u, result.FailureCount);
	}

	// endregion

	// region equality operators

	namespace {
		std::unordered_set<std::string> GetEqualTags() {
			return { "default", "copy" };
		}

		std::unordered_map<std::string, BatchUpdateResult> GenerateEqualityInstanceMap() {
			return {
				{ "default", { 77, 12, 98 } },
				{ "copy", { 77, 12, 98 } },

				{ "diff success", { 78, 12, 98 } },
				{ "diff neutral", { 77, 11, 98 } },
				{ "diff failure", { 77, 12, 90 } }
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

	// region to string

	TEST(TEST_CLASS, CanOutputResult) {
		// Arrange:
		auto result = BatchUpdateResult(77, 12, 98);

		// Act:
		auto str = test::ToString(result);

		// Assert:
		EXPECT_EQ("(success = 77, neutral = 12, failure = 98)", str);
	}

	// endregion
}}
