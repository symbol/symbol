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

#include "catapult/model/ReceiptSource.h"
#include "tests/test/nodeps/Alignment.h"
#include "tests/test/nodeps/Comparison.h"
#include "tests/TestHarness.h"

namespace catapult { namespace model {

#define TEST_CLASS ReceiptSourceTests

	// region size + alignment

#define RECEIPT_SOURCE_FIELDS FIELD(PrimaryId) FIELD(SecondaryId)

	TEST(TEST_CLASS, ReceiptSourceHasExpectedSize) {
		// Arrange:
		auto expectedSize = 0u;

#define FIELD(X) expectedSize += SizeOf32<decltype(ReceiptSource::X)>();
		RECEIPT_SOURCE_FIELDS
#undef FIELD

		// Assert:
		EXPECT_EQ(expectedSize, sizeof(ReceiptSource));
		EXPECT_EQ(8u, sizeof(ReceiptSource));
	}

	TEST(TEST_CLASS, ReceiptSourceHasProperAlignment) {
#define FIELD(X) EXPECT_ALIGNED(ReceiptSource, X);
		RECEIPT_SOURCE_FIELDS
#undef FIELD
	}

#undef RECEIPT_SOURCE_FIELDS

	// endregion

	// region constructor

	TEST(TEST_CLASS, CanCreateDefaultReceiptSource) {
		// Act:
		ReceiptSource source;

		// Assert:
		EXPECT_EQ(0u, source.PrimaryId);
		EXPECT_EQ(0u, source.SecondaryId);
	}

	TEST(TEST_CLASS, CanCreateReceiptSourceAroundSourceIds) {
		// Act:
		ReceiptSource source(17, 25);

		// Assert:
		EXPECT_EQ(17u, source.PrimaryId);
		EXPECT_EQ(25u, source.SecondaryId);
	}

	// endregion

	// region operators

	TEST(TEST_CLASS, OperatorLessThanReturnsTrueForSmallerValuesAndFalseOtherwise) {
		// Arrange:
		std::vector<ReceiptSource> sources{ { 11, 15 }, { 11, 16 }, { 12, 0 }, { 77, 77 }, { 88, 99 } };

		// Assert:
		test::AssertLessThanOperatorForEqualValues<ReceiptSource>({ 11, 15 }, { 11, 15 });
		test::AssertOperatorBehaviorForIncreasingValues(sources, std::less<>(), [](const auto& source) {
			std::ostringstream out;
			out << "(" << source.PrimaryId << "," << source.SecondaryId << ")";
			return out.str();
		});
	}

	// endregion
}}
