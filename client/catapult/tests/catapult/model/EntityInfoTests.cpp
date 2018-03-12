#include "catapult/model/EntityInfo.h"
#include "catapult/utils/MemoryUtils.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/test/nodeps/Equality.h"
#include "tests/TestHarness.h"

namespace catapult { namespace model {

#define TEST_CLASS EntityInfoTests

	// region general

	TEST(TEST_CLASS, CanCreateDefaultEntityInfo) {
		// Arrange + Act:
		EntityInfo<Block> entityInfo;

		// Assert: notice that hash is not zero-initialized by default constructor
		EXPECT_FALSE(!!entityInfo);
		EXPECT_FALSE(!!entityInfo.pEntity);
	}

	TEST(TEST_CLASS, CanCreateEntityInfo) {
		// Arrange:
		auto pBlock = utils::UniqueToShared(test::GenerateEmptyRandomBlock());
		auto hash = test::GenerateRandomData<Hash256_Size>();

		// Act:
		EntityInfo<Block> entityInfo(pBlock, hash);

		// Assert:
		EXPECT_TRUE(!!entityInfo);
		EXPECT_EQ(pBlock.get(), entityInfo.pEntity.get());
		EXPECT_EQ(hash, entityInfo.EntityHash);
	}

	TEST(TEST_CLASS, CanMoveConstructEntityInfo) {
		// Arrange:
		auto pBlock = utils::UniqueToShared(test::GenerateEmptyRandomBlock());
		auto hash = test::GenerateRandomData<Hash256_Size>();
		EntityInfo<Block> original(pBlock, hash);

		// Act:
		EntityInfo<Block> entityInfo(std::move(original));

		// Assert:
		EXPECT_TRUE(!!entityInfo);
		EXPECT_EQ(pBlock.get(), entityInfo.pEntity.get());
		EXPECT_EQ(hash, entityInfo.EntityHash);
	}

	// endregion

	// region hasher

	TEST(TEST_CLASS, EntityInfoHasher_SameObjectReturnsSameHash) {
		// Arrange:
		auto pBlock = utils::UniqueToShared(test::GenerateEmptyRandomBlock());
		EntityInfo<Block> entityInfo(pBlock, test::GenerateRandomData<Hash256_Size>());
		EntityInfoHasher<Block> hasher;

		// Act:
		auto result1 = hasher(entityInfo);
		auto result2 = hasher(entityInfo);

		// Assert:
		EXPECT_EQ(result1, result2);
	}

	TEST(TEST_CLASS, EntityInfoHasher_EqualObjectsReturnSameHash) {
		// Arrange:
		auto pBlock = utils::UniqueToShared(test::GenerateEmptyRandomBlock());
		auto hash = test::GenerateRandomData<Hash256_Size>();
		EntityInfo<Block> entityInfo1(pBlock, hash);
		EntityInfo<Block> entityInfo2(pBlock, hash);
		EntityInfoHasher<Block> hasher;

		// Act:
		auto result1 = hasher(entityInfo1);
		auto result2 = hasher(entityInfo2);

		// Assert:
		EXPECT_EQ(result1, result2);
	}

	TEST(TEST_CLASS, EntityInfoHasher_DifferentObjectsReturnDifferentHashes) {
		// Arrange:
		auto pBlock = utils::UniqueToShared(test::GenerateEmptyRandomBlock());
		EntityInfo<Block> entityInfo1(pBlock, test::GenerateRandomData<Hash256_Size>());
		EntityInfo<Block> entityInfo2(pBlock, test::GenerateRandomData<Hash256_Size>());
		EntityInfoHasher<Block> hasher;

		// Act:
		auto result1 = hasher(entityInfo1);
		auto result2 = hasher(entityInfo2);

		// Assert:
		EXPECT_NE(result1, result2);
	}

	// endregion

	// region comparer

	TEST(TEST_CLASS, EntityInfoComparer_ReturnsTrueForObjectsWithEqualHashes) {
		// Arrange:
		auto pBlock = utils::UniqueToShared(test::GenerateEmptyRandomBlock());
		auto hash = test::GenerateRandomData<Hash256_Size>();
		std::unordered_map<std::string, EntityInfo<Block>> map;
		map.emplace("default", EntityInfo<Block>(pBlock, hash));
		map.emplace("copy", EntityInfo<Block>(pBlock, hash));
		map.emplace("diff-block", EntityInfo<Block>(utils::UniqueToShared(test::GenerateEmptyRandomBlock()), hash));
		map.emplace("diff-hash", EntityInfo<Block>(pBlock, test::GenerateRandomData<Hash256_Size>()));

		// Assert:
		test::AssertEqualReturnsTrueForEqualObjects<EntityInfo<Block>>(
				"default",
				map,
				{ "default", "copy", "diff-block" },
				EntityInfoComparer<Block>());
	}

	// endregion

	// region transaction

	namespace {
		struct DetachedTransactionInfoTraits {
			using TransactionInfoType = DetachedTransactionInfo;

			static void AssertEmpty(const TransactionInfoType&) {
				// no additional fields to check
			}
		};

		struct TransactionInfoTraits {
			using TransactionInfoType = TransactionInfo;

			static void AssertEmpty(const TransactionInfoType& transactionInfo) {
				// Assert: check merkle component hash
				EXPECT_EQ(Hash256(), transactionInfo.MerkleComponentHash);
			}
		};
	}

#define TRANSACTION_INFO_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_Detached) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<DetachedTransactionInfoTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<TransactionInfoTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	TRANSACTION_INFO_TEST(CanCreateDefaultTransactionInfo) {
		// Act:
		typename TTraits::TransactionInfoType transactionInfo;

		// Assert: notice that hash(es) are not zero-initialized by default constructor
		EXPECT_FALSE(!!transactionInfo.pEntity);
		EXPECT_FALSE(!!transactionInfo.OptionalExtractedAddresses);
	}

	TRANSACTION_INFO_TEST(CanCreateTransactionInfoWithoutHash) {
		// Arrange:
		auto pTransaction = utils::UniqueToShared(test::GenerateRandomTransaction());

		// Act:
		typename TTraits::TransactionInfoType transactionInfo(pTransaction);

		// Assert:
		EXPECT_EQ(pTransaction.get(), transactionInfo.pEntity.get());
		EXPECT_EQ(Hash256(), transactionInfo.EntityHash);
		EXPECT_FALSE(!!transactionInfo.OptionalExtractedAddresses);
		TTraits::AssertEmpty(transactionInfo);
	}

	TRANSACTION_INFO_TEST(CanCreateTransactionInfoWithHash) {
		// Arrange:
		auto pTransaction = utils::UniqueToShared(test::GenerateRandomTransaction());
		auto entityHash = test::GenerateRandomData<Hash256_Size>();

		// Act:
		typename TTraits::TransactionInfoType transactionInfo(pTransaction, entityHash);

		// Assert:
		EXPECT_EQ(pTransaction.get(), transactionInfo.pEntity.get());
		EXPECT_EQ(entityHash, transactionInfo.EntityHash);
		EXPECT_FALSE(!!transactionInfo.OptionalExtractedAddresses);
		TTraits::AssertEmpty(transactionInfo);
	}

	TEST(TEST_CLASS, CanCopyDetachedTransactionInfo) {
		// Arrange:
		auto pTransaction = utils::UniqueToShared(test::GenerateRandomTransaction());
		auto entityHash = test::GenerateRandomData<Hash256_Size>();
		auto pExtractedAddress = std::make_shared<AddressSet>();

		DetachedTransactionInfo transactionInfo(pTransaction, entityHash);
		transactionInfo.OptionalExtractedAddresses = pExtractedAddress;

		// Act:
		auto transactionInfoCopy = transactionInfo.copy();

		// Assert:
		// - original info is unmodified
		EXPECT_EQ(pTransaction.get(), transactionInfo.pEntity.get());
		EXPECT_EQ(entityHash, transactionInfo.EntityHash);
		EXPECT_EQ(pExtractedAddress.get(), transactionInfo.OptionalExtractedAddresses.get());

		// - copied info has correct values
		EXPECT_EQ(pTransaction.get(), transactionInfoCopy.pEntity.get());
		EXPECT_EQ(entityHash, transactionInfoCopy.EntityHash);
		EXPECT_EQ(pExtractedAddress.get(), transactionInfoCopy.OptionalExtractedAddresses.get());
	}

	TEST(TEST_CLASS, CanCopyTransactionInfo) {
		// Arrange:
		auto pTransaction = utils::UniqueToShared(test::GenerateRandomTransaction());
		auto entityHash = test::GenerateRandomData<Hash256_Size>();
		auto pExtractedAddress = std::make_shared<AddressSet>();
		auto merkleComponentHash = test::GenerateRandomData<Hash256_Size>();

		TransactionInfo transactionInfo(pTransaction, entityHash);
		transactionInfo.OptionalExtractedAddresses = pExtractedAddress;
		transactionInfo.MerkleComponentHash = merkleComponentHash;

		// Act:
		auto transactionInfoCopy = transactionInfo.copy();

		// Assert:
		// - original info is unmodified
		EXPECT_EQ(pTransaction.get(), transactionInfo.pEntity.get());
		EXPECT_EQ(entityHash, transactionInfo.EntityHash);
		EXPECT_EQ(pExtractedAddress.get(), transactionInfo.OptionalExtractedAddresses.get());
		EXPECT_EQ(merkleComponentHash, transactionInfo.MerkleComponentHash);

		// - copied info has correct values
		EXPECT_EQ(pTransaction.get(), transactionInfoCopy.pEntity.get());
		EXPECT_EQ(entityHash, transactionInfoCopy.EntityHash);
		EXPECT_EQ(pExtractedAddress.get(), transactionInfoCopy.OptionalExtractedAddresses.get());
		EXPECT_EQ(merkleComponentHash, transactionInfoCopy.MerkleComponentHash);
	}

	// endregion
}}
