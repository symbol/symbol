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

#include "catapult/model/AnnotatedEntityRange.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace model {

#define TEST_CLASS AnnotatedEntityRangeTests

	TEST(TEST_CLASS, CanCreateDefaultRange) {
		// Act:
		auto annotatedRange = AnnotatedEntityRange<Block>();

		// Assert:
		EXPECT_TRUE(annotatedRange.Range.empty());
		EXPECT_EQ(Key(), annotatedRange.SourceIdentity.PublicKey);
		EXPECT_EQ("", annotatedRange.SourceIdentity.Host);
	}

	TEST(TEST_CLASS, CanCreateAroundRange) {
		// Arrange:
		auto range = test::CreateBlockEntityRange(3);
		const auto* pRangeData = range.data();

		// Act:
		auto annotatedRange = AnnotatedEntityRange<Block>(std::move(range));

		// Assert:
		EXPECT_EQ(pRangeData, annotatedRange.Range.data());
		EXPECT_EQ(Key(), annotatedRange.SourceIdentity.PublicKey);
		EXPECT_EQ("", annotatedRange.SourceIdentity.Host);
	}

	TEST(TEST_CLASS, CanCreateAroundRangeAndContext) {
		// Arrange:
		auto sourcePublicKey = test::GenerateRandomByteArray<Key>();
		auto range = test::CreateBlockEntityRange(3);
		const auto* pRangeData = range.data();

		// Act:
		auto annotatedRange = AnnotatedEntityRange<Block>(std::move(range), { sourcePublicKey, "11.22.33.44" });

		// Assert:
		EXPECT_EQ(pRangeData, annotatedRange.Range.data());
		EXPECT_EQ(sourcePublicKey, annotatedRange.SourceIdentity.PublicKey);
		EXPECT_EQ("11.22.33.44", annotatedRange.SourceIdentity.Host);
	}
}}
