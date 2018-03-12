#include "catapult/model/TransactionStatus.h"
#include "tests/test/nodeps/Equality.h"
#include "tests/TestHarness.h"

namespace catapult { namespace model {

#define TEST_CLASS TransactionStatusTests

	TEST(TEST_CLASS, CanCreateTransactionStatus) {
		// Arrange + Act:
		Hash256 hash = test::GenerateRandomData<Hash256_Size>();
		TransactionStatus result(hash, 123, Timestamp(234));

		// Assert:
		EXPECT_EQ(Hash256_Size + sizeof(uint32_t) + sizeof(Timestamp), sizeof(result));
		EXPECT_EQ(hash, result.Hash);
		EXPECT_EQ(123u, result.Status);
		EXPECT_EQ(Timestamp(234), result.Deadline);
	}

	// region equality operators

	namespace {
		std::unordered_map<std::string, TransactionStatus> GenerateEqualityInstanceMap() {
			auto hash1 = test::GenerateRandomData<Hash256_Size>();
			auto hash2 = test::GenerateRandomData<Hash256_Size>();
			return {
				{ "default", TransactionStatus(hash1, 123, Timestamp(234)) },
				{ "copy", TransactionStatus(hash1, 123, Timestamp(234)) },
				{ "diff-hash", TransactionStatus(hash2, 123, Timestamp(234)) },
				{ "diff-status", TransactionStatus(hash1, 234, Timestamp(234)) },
				{ "diff-deadline", TransactionStatus(hash1, 123, Timestamp(345)) }
			};
		}

		std::unordered_set<std::string> GetEqualTags() {
			return { "default", "copy", "diff-status", "diff-deadline" };
		}
	}

	TEST(TEST_CLASS, OperatorEqualReturnsTrueOnlyForEqualValues) {
		// Assert:
		test::AssertOperatorEqualReturnsTrueForEqualObjects("default", GenerateEqualityInstanceMap(), GetEqualTags());
	}

	TEST(TEST_CLASS, OperatorNotEqualReturnsTrueOnlyForUnequalValues) {
		// Assert:
		test::AssertOperatorNotEqualReturnsTrueForUnequalObjects("default", GenerateEqualityInstanceMap(), GetEqualTags());
	}

	// endregion
}}
