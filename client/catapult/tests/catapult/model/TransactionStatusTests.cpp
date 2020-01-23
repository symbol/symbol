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

#include "catapult/model/TransactionStatus.h"
#include "tests/test/nodeps/Alignment.h"
#include "tests/test/nodeps/Equality.h"
#include "tests/TestHarness.h"

namespace catapult { namespace model {

#define TEST_CLASS TransactionStatusTests

	// region size + alignment

#define TRANSACTION_STATUS_FIELDS FIELD(Hash) FIELD(Deadline) FIELD(Status)

	TEST(TEST_CLASS, TransactionStatusHasExpectedSize) {
		// Arrange:
		auto expectedSize = 0u;

#define FIELD(X) expectedSize += sizeof(TransactionStatus::X);
		TRANSACTION_STATUS_FIELDS
#undef FIELD

		// Assert:
		EXPECT_EQ(expectedSize, sizeof(TransactionStatus));
		EXPECT_EQ(44u, sizeof(TransactionStatus));
	}

	TEST(TEST_CLASS, TransactionStatusHasProperAlignment) {
#define FIELD(X) EXPECT_ALIGNED(TransactionStatus, X);
		TRANSACTION_STATUS_FIELDS
#undef FIELD
	}

#undef TRANSACTION_STATUS_FIELDS

	// endregion

	// region constructor

	TEST(TEST_CLASS, CanCreateTransactionStatus) {
		// Arrange + Act:
		Hash256 hash = test::GenerateRandomByteArray<Hash256>();
		TransactionStatus result(hash, Timestamp(234), 123);

		// Assert:
		EXPECT_EQ(Hash256::Size + sizeof(uint32_t) + sizeof(Timestamp), sizeof(result));
		EXPECT_EQ(hash, result.Hash);
		EXPECT_EQ(Timestamp(234), result.Deadline);
		EXPECT_EQ(123u, result.Status);
	}

	// endregion

	// region equality operators

	namespace {
		std::unordered_map<std::string, TransactionStatus> GenerateEqualityInstanceMap() {
			auto hash1 = test::GenerateRandomByteArray<Hash256>();
			auto hash2 = test::GenerateRandomByteArray<Hash256>();
			return {
				{ "default", TransactionStatus(hash1, Timestamp(234), 123) },
				{ "copy", TransactionStatus(hash1, Timestamp(234), 123) },
				{ "diff-hash", TransactionStatus(hash2, Timestamp(234), 123) },
				{ "diff-deadline", TransactionStatus(hash1, Timestamp(345), 123) },
				{ "diff-status", TransactionStatus(hash1, Timestamp(234), 234) }
			};
		}

		std::unordered_set<std::string> GetEqualTags() {
			return { "default", "copy", "diff-status", "diff-deadline" };
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
