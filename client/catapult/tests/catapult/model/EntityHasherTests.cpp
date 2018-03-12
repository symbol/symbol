#include "catapult/model/EntityHasher.h"
#include "catapult/crypto/Hashes.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/test/core/mocks/MockTransaction.h"
#include "tests/test/core/mocks/MockTransactionPluginWithCustomBuffers.h"
#include "tests/TestHarness.h"
#include <array>

namespace catapult { namespace model {

#define TEST_CLASS EntityHasherTests

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

	// region CalculateHash - basic

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

	// region CalculateHash - block

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

	// region CalculateHash - transaction

	TEST(TEST_CLASS, CalculateTransactionHashReturnsExpectedHash) {
		// Arrange: create a predefined transaction
		auto pTransaction = test::GenerateDeterministicTransaction();

		// Act:
		auto hash = CalculateHash(*pTransaction);

		// Assert:
		EXPECT_EQ(test::Deterministic_Transaction_Hash_String, test::ToHexString(hash));
	}

	// endregion

	// region CalculateHash - verifiable entity

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

	// region CalculateMerkleComponentHash (transaction)

	TEST(TEST_CLASS, CalculateMerkleComponentHash_ReturnsTransactionHashWhenThereAreNoSupplementaryBuffers) {
		// Arrange:
		auto pPlugin = mocks::CreateMockTransactionPluginWithCustomBuffers(mocks::OffsetRange{ 5, 15 }, {});
		auto registry = TransactionRegistry();
		registry.registerPlugin(std::move(pPlugin));

		auto pTransaction = test::GenerateRandomTransaction();
		const auto& transaction = *pTransaction;
		auto transactionHash = test::GenerateRandomData<Hash256_Size>();

		// Act:
		auto merkleComponentHash = CalculateMerkleComponentHash(transaction, transactionHash, registry);

		// Assert:
		EXPECT_EQ(transactionHash, merkleComponentHash);
	}

	TEST(TEST_CLASS, CalculateMerkleComponentHash_IsDependentOnMerkleSupplementaryBuffers) {
		// Arrange:
		auto pPlugin = mocks::CreateMockTransactionPluginWithCustomBuffers(
				mocks::OffsetRange{ 6, 10 },
				std::vector<mocks::OffsetRange>{ { 7, 11 }, { 4, 7 }, { 12, 20 } });
		auto registry = TransactionRegistry();
		registry.registerPlugin(std::move(pPlugin));

		auto pTransaction = test::GenerateRandomTransaction();
		const auto& transaction = *pTransaction;
		auto transactionHash = test::GenerateRandomData<Hash256_Size>();

		Hash256 expectedMerkleComponentHash;
		crypto::Sha3_256_Builder sha3;
		sha3.update(transactionHash);
		sha3.update(mocks::ExtractBuffer({ 7, 11 }, &transaction));
		sha3.update(mocks::ExtractBuffer({ 4, 7 }, &transaction));
		sha3.update(mocks::ExtractBuffer({ 12, 20 }, &transaction));
		sha3.final(expectedMerkleComponentHash);

		// Act:
		auto merkleComponentHash = CalculateMerkleComponentHash(transaction, transactionHash, registry);

		// Assert:
		EXPECT_EQ(expectedMerkleComponentHash, merkleComponentHash);
		EXPECT_NE(transactionHash, merkleComponentHash);
	}

	// endregion

	// region UpdateHashes (transaction element)

	TEST(TEST_CLASS, UpdateHashes_TransactionEntityHashIsDependentOnDataBuffer) {
		// Arrange:
		auto pPlugin = mocks::CreateMockTransactionPluginWithCustomBuffers(mocks::OffsetRange{ 5, 15 }, {});
		auto registry = TransactionRegistry();
		registry.registerPlugin(std::move(pPlugin));

		auto pTransaction = test::GenerateRandomTransaction();
		auto transactionElement = TransactionElement(*pTransaction);
		const auto& transaction = *pTransaction;

		// - since there are no supplementary buffers, the transaction hash is equal to the merkle hash
		auto expectedEntityHash = CalculateHash(transaction, mocks::ExtractBuffer({ 5, 15 }, &transaction));

		// Act:
		UpdateHashes(registry, transactionElement);

		// Assert:
		EXPECT_EQ(expectedEntityHash, transactionElement.EntityHash);
		EXPECT_EQ(expectedEntityHash, transactionElement.MerkleComponentHash);
		EXPECT_EQ(transactionElement.EntityHash, transactionElement.MerkleComponentHash);
	}

	TEST(TEST_CLASS, UpdateHashes_TransactionMerkleComponentHashIsDependentOnMerkleSupplementaryBuffers) {
		// Arrange:
		auto pPlugin = mocks::CreateMockTransactionPluginWithCustomBuffers(
				mocks::OffsetRange{ 6, 10 },
				std::vector<mocks::OffsetRange>{ { 7, 11 }, { 4, 7 }, { 12, 20 } });
		auto registry = TransactionRegistry();
		registry.registerPlugin(std::move(pPlugin));

		auto pTransaction = test::GenerateRandomTransaction();
		auto transactionElement = TransactionElement(*pTransaction);
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
		UpdateHashes(registry, transactionElement);

		// Assert:
		EXPECT_EQ(expectedEntityHash, transactionElement.EntityHash);
		EXPECT_EQ(expectedMerkleComponentHash, transactionElement.MerkleComponentHash);
		EXPECT_NE(transactionElement.EntityHash, transactionElement.MerkleComponentHash);
	}

	// endregion
}}
