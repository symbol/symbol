#include "catapult/model/EntityInfo.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace model {

	namespace {
		template<typename T>
		std::shared_ptr<T> UniqueToShared(std::unique_ptr<T>&& pEntity) {
			return std::shared_ptr<T>(std::move(pEntity));
		}
	}

	// region general

	TEST(EntityInfoTests, CanCreateDefaultEntityInfo) {
		// Arrange + Act:
		EntityInfo<model::Block> entityInfo;

		// Assert:
		EXPECT_FALSE(!!entityInfo.pEntity);
	}

	TEST(EntityInfoTests, CanCreateEntityInfo) {
		// Arrange:
		auto pBlock = UniqueToShared(test::GenerateEmptyRandomBlock());
		auto hash = test::GenerateRandomData<Hash256_Size>();

		// Act:
		EntityInfo<model::Block> entityInfo(pBlock, hash);

		// Assert:
		EXPECT_EQ(pBlock.get(), entityInfo.pEntity.get());
		EXPECT_EQ(hash, entityInfo.EntityHash);
	}

	TEST(EntityInfoTests, CanMoveConstructEntityInfo) {
		// Arrange:
		auto pBlock = UniqueToShared(test::GenerateEmptyRandomBlock());
		auto hash = test::GenerateRandomData<Hash256_Size>();
		EntityInfo<model::Block> original(pBlock, hash);

		// Act:
		EntityInfo<model::Block> entityInfo(std::move(original));

		// Assert:
		EXPECT_EQ(pBlock.get(), entityInfo.pEntity.get());
		EXPECT_EQ(hash, entityInfo.EntityHash);
	}

	// endregion

	// region transaction

	TEST(EntityInfoTests, CanCreateTransactionInfoWithoutMetadata) {
		// Arrange:
		auto pTransaction = UniqueToShared(test::GenerateRandomTransaction());

		// Act:
		TransactionInfo transactionInfo(pTransaction);

		// Assert:
		EXPECT_EQ(pTransaction.get(), transactionInfo.pEntity.get());
		EXPECT_EQ(Hash256(), transactionInfo.EntityHash);
		EXPECT_EQ(Hash256(), transactionInfo.MerkleComponentHash);
	}

	TEST(EntityInfoTests, CanCreateTransactionInfo) {
		// Arrange:
		auto pTransaction = UniqueToShared(test::GenerateRandomTransaction());
		auto entityHash = test::GenerateRandomData<Hash256_Size>();
		auto merkleComponentHash = test::GenerateRandomData<Hash256_Size>();

		// Act:
		TransactionInfo transactionInfo(pTransaction, entityHash, merkleComponentHash);

		// Assert:
		EXPECT_EQ(pTransaction.get(), transactionInfo.pEntity.get());
		EXPECT_EQ(entityHash, transactionInfo.EntityHash);
		EXPECT_EQ(merkleComponentHash, transactionInfo.MerkleComponentHash);
	}

	TEST(EntityInfoTests, CanCopyTransactionInfo) {
		// Arrange:
		auto pTransaction = UniqueToShared(test::GenerateRandomTransaction());
		auto entityHash = test::GenerateRandomData<Hash256_Size>();
		auto merkleComponentHash = test::GenerateRandomData<Hash256_Size>();
		TransactionInfo transactionInfo(pTransaction, entityHash, merkleComponentHash);

		// Act:
		auto copiedTransactionInfo = transactionInfo.copy();

		// Assert:
		// - original info is unmodified
		EXPECT_EQ(pTransaction.get(), transactionInfo.pEntity.get());
		EXPECT_EQ(entityHash, transactionInfo.EntityHash);
		EXPECT_EQ(merkleComponentHash, transactionInfo.MerkleComponentHash);

		// - copied info has correct values
		EXPECT_EQ(pTransaction.get(), copiedTransactionInfo.pEntity.get());
		EXPECT_EQ(entityHash, copiedTransactionInfo.EntityHash);
		EXPECT_EQ(merkleComponentHash, copiedTransactionInfo.MerkleComponentHash);
	}

	// endregion
}}
