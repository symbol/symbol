#include "catapult/model/Transaction.h"
#include "tests/test/core/TransactionTestUtils.h"
#include "tests/test/core/mocks/MockTransaction.h"
#include "tests/TestHarness.h"

namespace catapult { namespace model {

#define TEST_CLASS TransactionTests

	// region structure + size

	TEST(TEST_CLASS, StructureInheritancePreservesLayout) {
		// Assert: the derived fields should start where the base fields end
		ASSERT_EQ(sizeof(VerifiableEntity), offsetof(Transaction, Fee));
	}

	TEST(TEST_CLASS, EntityHasExpectedSize) {
		// Arrange:
		auto expectedSize = sizeof(VerifiableEntity) // base
			+ sizeof(uint64_t) // fee
			+ sizeof(uint64_t); // deadline

		// Assert:
		EXPECT_EQ(expectedSize, sizeof(Transaction));
		EXPECT_EQ(104u + 16u, sizeof(Transaction));
	}

	// endregion

	// region IsSizeValid

	namespace {
		bool IsSizeValid(const Transaction& transaction) {
			auto pRegistry = mocks::CreateDefaultTransactionRegistry();
			return IsSizeValid(transaction, *pRegistry);
		}
	}

	TEST(TEST_CLASS, SizeIsInvalidForTransactionWithUnknownType) {
		// Arrange:
		Transaction transaction;
		transaction.Type = static_cast<EntityType>(-1);
		transaction.Size = sizeof(Transaction);

		// Act + Assert:
		EXPECT_FALSE(IsSizeValid(transaction));
	}

	namespace {
		std::unique_ptr<Transaction> CreateMockTransaction(uint32_t delta) {
			auto pTransaction = mocks::CreateMockTransaction(7);
			pTransaction->Size += delta;
			return std::move(pTransaction);
		}
	}

	TEST(TEST_CLASS, SizeIsValidForTransactionWithEqualReportedSizeAndActualSize) {
		// Arrange:
		auto pTransaction = CreateMockTransaction(0);

		// Act + Assert:
		EXPECT_TRUE(IsSizeValid(*pTransaction));
	}

	TEST(TEST_CLASS, SizeIsInvalidForTransactionWithReportedSizeLessThanActualSize) {
		// Arrange:
		auto pTransaction = CreateMockTransaction(static_cast<uint32_t>(-1));

		// Act:
		EXPECT_FALSE(IsSizeValid(*pTransaction));
	}

	TEST(TEST_CLASS, SizeIsInvalidForTransactionWithReportedSizeGreaterThanActualSize) {
		// Arrange:
		auto pTransaction = CreateMockTransaction(1);

		// Act + Assert:
		EXPECT_FALSE(IsSizeValid(*pTransaction));
	}

	// endregion
}}
