#include "catapult/model/EntityHasher.h"
#include "catapult/crypto/Hashes.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/test/core/mocks/MockTransaction.h"
#include "tests/test/core/mocks/MockTransactionPluginWithCustomBuffers.h"
#include "tests/TestHarness.h"
#include <array>

#define TEST_CLASS EntityHasherTests

namespace catapult { namespace model {

	namespace {
		struct BlockTraits {
			static std::unique_ptr<Block> Generate() {
				return test::GenerateBlockWithTransactionsAtHeight(7, 7);
			}

			static Hash256 CalculateHash(const Block& block) {
				return model::CalculateHash(block);
			}
		};

		struct TransactionTraits {
			static std::unique_ptr<Transaction> Generate() {
				return test::GenerateRandomTransaction();
			}

			static Hash256 CalculateHash(const Transaction& transaction) {
				return model::CalculateHash(transaction);
			}
		};

		struct VerifiableEntityTraits {
			static std::unique_ptr<VerifiableEntity> Generate() {
				auto pEntity = std::make_unique<VerifiableEntity>();
				test::FillWithRandomData({ reinterpret_cast<uint8_t*>(pEntity.get()), sizeof(VerifiableEntity) });
				return pEntity;
			}

			static Hash256 CalculateHash(const VerifiableEntity& entity) {
				// hash full entity header body in traits-based tests
				auto headerSize = VerifiableEntity::Header_Size;
				const auto* pEntityData = reinterpret_cast<const uint8_t*>(&entity);
				return model::CalculateHash(entity, { pEntityData + headerSize, sizeof(VerifiableEntity) - headerSize });
			}
		};
	}

	// region basic

#define BASIC_HASH_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_Block) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<BlockTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Transaction) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<TransactionTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_VerifiableEntity) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<VerifiableEntityTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	BASIC_HASH_TEST(HashChangesIfRPartOfSignatureChanges) {
		// Arrange:
		auto pEntity = TTraits::Generate();
		auto originalHash = TTraits::CalculateHash(*pEntity);

		// Act:
		pEntity->Signature[0] ^= 0xFF;
		auto modifiedHash = TTraits::CalculateHash(*pEntity);

		// Assert:
		EXPECT_NE(originalHash, modifiedHash);
	}

	BASIC_HASH_TEST(HashDoesNotChangeIfSPartOfSignatureChanges) {
		// Arrange:
		auto pEntity = TTraits::Generate();
		auto originalHash = TTraits::CalculateHash(*pEntity);

		// Act:
		pEntity->Signature[Signature_Size / 2] ^= 0xFF;
		auto modifiedHash = TTraits::CalculateHash(*pEntity);

		// Assert:
		EXPECT_EQ(originalHash, modifiedHash);
	}

	BASIC_HASH_TEST(HashChangesIfSignerChanges) {
		// Arrange:
		auto pEntity = TTraits::Generate();
		auto originalHash = TTraits::CalculateHash(*pEntity);

		// Act:
		pEntity->Signer[Key_Size / 2] ^= 0xFF;
		auto modifiedHash = TTraits::CalculateHash(*pEntity);

		// Assert:
		EXPECT_NE(originalHash, modifiedHash);
	}

	BASIC_HASH_TEST(HashChangesIfEntityDataChanges) {
		// Arrange:
		auto pEntity = TTraits::Generate();
		auto originalHash = TTraits::CalculateHash(*pEntity);

		// Act: change the last byte
		auto* pLastByte = reinterpret_cast<uint8_t*>(pEntity.get() + 1) - 1;
		++*pLastByte;
		auto modifiedHash = TTraits::CalculateHash(*pEntity);

		// Assert:
		EXPECT_NE(originalHash, modifiedHash);
	}

	// endregion

	// region block

	TEST(TEST_CLASS, CalculateBlockHashReturnsExpectedHash) {
		// Arrange: create a predefined block with one predefined transaction
		auto pBlock = test::GenerateDeterministicBlock();

		// Act:
		auto hash = CalculateHash(*pBlock);

		// Assert:
		EXPECT_EQ(test::Deterministic_Block_Hash_String, test::ToHexString(hash));
	}

	TEST(TEST_CLASS, BlockHashDoesNotChangeIfBlockTransactionDataChanges) {
		// Arrange:
		auto pBlock = BlockTraits::Generate();
		auto originalHash = CalculateHash(*pBlock);

		// Act: change a transaction deadline
		//     (notice that in a properly constructed block, this change will cause the BlockTransactionsHash to change
		//      in this test, that field is not set so the before and after hashes are equal)
		pBlock->TransactionsPtr()->Deadline = pBlock->TransactionsPtr()->Deadline + Timestamp(1);
		auto modifiedHash = CalculateHash(*pBlock);

		// Assert:
		EXPECT_EQ(originalHash, modifiedHash);
	}

	// endregion

	// region transaction

	TEST(TEST_CLASS, CalculateTransactionHashReturnsExpectedHash) {
		// Arrange: create a predefined transaction
		auto pTransaction = test::GenerateDeterministicTransaction();

		// Act:
		auto hash = CalculateHash(*pTransaction);

		// Assert:
		EXPECT_EQ(test::Deterministic_Transaction_Hash_String, test::ToHexString(hash));
	}

	// endregion

	// region verifiable entity

	TEST(TEST_CLASS, VerifiableEntityHashChangesIfDataBufferDataChanges) {
		// Arrange:
		auto pEntity = VerifiableEntityTraits::Generate();
		const auto* pEntityData = reinterpret_cast<uint8_t*>(pEntity.get());
		auto originalHash = CalculateHash(*pEntity, { pEntityData, sizeof(VerifiableEntity) - 1});

		// Act:
		auto modifiedHash = CalculateHash(*pEntity, { pEntityData + 1, sizeof(VerifiableEntity) - 1 });

		// Assert:
		EXPECT_NE(originalHash, modifiedHash);
	}

	TEST(TEST_CLASS, VerifiableEntityHashChangesIfDataBufferSizeChanges) {
		// Arrange:
		auto pEntity = VerifiableEntityTraits::Generate();
		const auto* pEntityData = reinterpret_cast<uint8_t*>(pEntity.get());
		auto originalHash = CalculateHash(*pEntity, { pEntityData, sizeof(VerifiableEntity) });

		// Act:
		auto modifiedHash = CalculateHash(*pEntity, { pEntityData, sizeof(VerifiableEntity) - 1 });

		// Assert:
		EXPECT_NE(originalHash, modifiedHash);
	}

	// endregion

	// region transaction element

	TEST(TEST_CLASS, UpdateHashes_TransactionEntityHashIsDependentOnDataBuffer) {
		// Arrange:
		auto pPlugin = mocks::CreateMockTransactionPluginWithCustomBuffers(mocks::OffsetRange{ 5, 15 }, {});
		auto registry = TransactionRegistry();
		registry.registerPlugin(std::move(pPlugin));

		auto pTransaction = test::GenerateRandomTransaction();
		auto txElement = TransactionElement(*pTransaction);
		const auto& transaction = *pTransaction;

		// - since there are no supplementary buffers, the transaction hash is equal to the merkle hash
		auto expectedEntityHash = CalculateHash(transaction, mocks::ExtractBuffer({ 5, 15 }, &transaction));

		// Act:
		UpdateHashes(registry, txElement);

		// Assert:
		EXPECT_EQ(expectedEntityHash, txElement.EntityHash);
		EXPECT_EQ(expectedEntityHash, txElement.MerkleComponentHash);
		EXPECT_EQ(txElement.EntityHash, txElement.MerkleComponentHash);
	}

	TEST(TEST_CLASS, UpdateHashes_TransactionMerkleComponentHashIsDependentOnMerkleSupplementaryBuffers) {
		// Arrange:
		auto pPlugin = mocks::CreateMockTransactionPluginWithCustomBuffers(
				mocks::OffsetRange{ 6, 10 },
				std::vector<mocks::OffsetRange>{ { 7, 11 }, { 4, 7 }, { 12, 20 } });
		auto registry = TransactionRegistry();
		registry.registerPlugin(std::move(pPlugin));

		auto pTransaction = test::GenerateRandomTransaction();
		auto txElement = TransactionElement(*pTransaction);
		const auto& transaction = *pTransaction;

		auto expectedEntityHash = CalculateHash(transaction, mocks::ExtractBuffer({ 6, 10 }, &transaction));

		Hash256 expectedMerkleComponentHash;
		crypto::Sha3_256_Builder sha3;
		sha3.update(expectedEntityHash);
		sha3.update(mocks::ExtractBuffer({ 7, 11 }, &transaction));
		sha3.update(mocks::ExtractBuffer({ 4, 7 }, &transaction));
		sha3.update(mocks::ExtractBuffer({ 12, 20 }, &transaction));
		sha3.final(expectedMerkleComponentHash);

		// Act:
		UpdateHashes(registry, txElement);

		// Assert:
		EXPECT_EQ(expectedEntityHash, txElement.EntityHash);
		EXPECT_EQ(expectedMerkleComponentHash, txElement.MerkleComponentHash);
		EXPECT_NE(txElement.EntityHash, txElement.MerkleComponentHash);
	}

	// endregion
}}
