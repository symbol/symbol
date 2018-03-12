#include "sync/src/PredicateUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace sync {

#define TEST_CLASS PredicateUtilsTests

	TEST(TEST_CLASS, ToUnknownTransactionPredicateDelegatesToKnownHashPredicateForTransactionType) {
		// Arrange:
		auto entityType = model::BasicEntityType::Transaction;
		auto knownHash = test::GenerateRandomData<Hash256_Size>();
		auto otherHash = test::GenerateRandomData<Hash256_Size>();
		auto predicate = ToUnknownTransactionPredicate([&knownHash](auto timestamp, const auto& hash) {
			return Timestamp(123) == timestamp && knownHash == hash;
		});

		// Act + Assert:
		EXPECT_FALSE(predicate(entityType, Timestamp(123), knownHash)); // match
		EXPECT_TRUE(predicate(entityType, Timestamp(245), knownHash)); // diff timestamp
		EXPECT_TRUE(predicate(entityType, Timestamp(123), otherHash)); // diff hash
		EXPECT_TRUE(predicate(entityType, Timestamp(245), otherHash)); // diff timestamp + hash
	}

	TEST(TEST_CLASS, ToUnknownTransactionPredicateAlwaysReturnsFalseForNonTransactionType) {
		// Arrange:
		auto entityType = model::BasicEntityType::Block;
		auto knownHash = test::GenerateRandomData<Hash256_Size>();
		auto otherHash = test::GenerateRandomData<Hash256_Size>();
		auto predicate = ToUnknownTransactionPredicate([&knownHash](auto timestamp, const auto& hash) {
			return Timestamp(123) == timestamp && knownHash == hash;
		});

		// Act + Assert:
		EXPECT_FALSE(predicate(entityType, Timestamp(123), knownHash)); // match
		EXPECT_FALSE(predicate(entityType, Timestamp(245), knownHash)); // diff timestamp
		EXPECT_FALSE(predicate(entityType, Timestamp(123), otherHash)); // diff hash
		EXPECT_FALSE(predicate(entityType, Timestamp(245), otherHash)); // diff timestamp + hash
	}
}}
