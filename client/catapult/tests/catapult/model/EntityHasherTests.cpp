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

#include "catapult/model/EntityHasher.h"
#include "catapult/crypto/Hashes.h"
#include "catapult/crypto/MerkleHashBuilder.h"
#include "catapult/utils/HexParser.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/test/core/mocks/MockTransaction.h"
#include "tests/test/core/mocks/MockTransactionPluginWithCustomBuffers.h"
#include "tests/test/nodeps/TestConstants.h"
#include "tests/TestHarness.h"
#include <array>

namespace catapult { namespace model {

#define TEST_CLASS EntityHasherTests

	namespace {
		struct BlockNormalTraits {
			static constexpr auto Footer_Size = PaddedBlockFooter::Footer_Size;

			static std::unique_ptr<Block> Generate() {
				return test::GenerateBlockWithTransactions(7);
			}

			static Hash256 CalculateHash(const Block& block, const GenerationHashSeed&) {
				return model::CalculateHash(block);
			}
		};

		struct BlockImportanceTraits {
			static constexpr auto Footer_Size = 0u;

			static std::unique_ptr<Block> Generate() {
				return test::GenerateImportanceBlockWithTransactions(7);
			}

			static Hash256 CalculateHash(const Block& block, const GenerationHashSeed&) {
				return model::CalculateHash(block);
			}
		};

		struct TransactionTraits {
			static constexpr auto Footer_Size = 0u;

			static std::unique_ptr<Transaction> Generate() {
				return test::GenerateRandomTransaction();
			}

			static Hash256 CalculateHash(const Transaction& transaction, const GenerationHashSeed& generationHashSeed) {
				return model::CalculateHash(transaction, generationHashSeed);
			}
		};

		struct TransactionCustomPayloadTraits : public TransactionTraits {
			static Hash256 CalculateHash(const Transaction& transaction, const GenerationHashSeed& generationHashSeed) {
				// hash full transaction header body in traits-based tests
				auto transactionBuffer = RawBuffer{
					reinterpret_cast<const uint8_t*>(&transaction) + Transaction::Header_Size,
					sizeof(Transaction) - Transaction::Header_Size
				};
				return model::CalculateHash(transaction, generationHashSeed, transactionBuffer);
			}
		};
	}

	// region CalculateHash - basic

#define BASIC_HASH_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_BlockNormal) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<BlockNormalTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_BlockImportance) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<BlockImportanceTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Transaction) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<TransactionTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_TransactionCustomPayload) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<TransactionCustomPayloadTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	BASIC_HASH_TEST(HashChangesWhenRPartOfSignatureChanges) {
		// Arrange:
		auto pEntity = TTraits::Generate();
		auto generationHashSeed = test::GenerateRandomByteArray<GenerationHashSeed>();
		auto originalHash = TTraits::CalculateHash(*pEntity, generationHashSeed);

		// Act:
		pEntity->Signature[0] ^= 0xFF;
		auto modifiedHash = TTraits::CalculateHash(*pEntity, generationHashSeed);

		// Assert:
		EXPECT_NE(originalHash, modifiedHash);
	}

	BASIC_HASH_TEST(HashChangesWhenSPartOfSignatureChanges) {
		// Arrange:
		auto pEntity = TTraits::Generate();
		auto generationHashSeed = test::GenerateRandomByteArray<GenerationHashSeed>();
		auto originalHash = TTraits::CalculateHash(*pEntity, generationHashSeed);

		// Act:
		pEntity->Signature[Signature::Size / 2] ^= 0xFF;
		auto modifiedHash = TTraits::CalculateHash(*pEntity, generationHashSeed);

		// Assert:
		EXPECT_NE(originalHash, modifiedHash);
	}

	BASIC_HASH_TEST(HashChangesWhenSignerChanges) {
		// Arrange:
		auto pEntity = TTraits::Generate();
		auto generationHashSeed = test::GenerateRandomByteArray<GenerationHashSeed>();
		auto originalHash = TTraits::CalculateHash(*pEntity, generationHashSeed);

		// Act:
		pEntity->SignerPublicKey[Key::Size / 2] ^= 0xFF;
		auto modifiedHash = TTraits::CalculateHash(*pEntity, generationHashSeed);

		// Assert:
		EXPECT_NE(originalHash, modifiedHash);
	}

	BASIC_HASH_TEST(HashChangesWhenEntityDataChanges) {
		// Arrange:
		auto pEntity = TTraits::Generate();
		auto generationHashSeed = test::GenerateRandomByteArray<GenerationHashSeed>();
		auto originalHash = TTraits::CalculateHash(*pEntity, generationHashSeed);

		// Act: change the last byte
		auto* pLastByte = reinterpret_cast<uint8_t*>(pEntity.get() + 1) - TTraits::Footer_Size - 1;
		*pLastByte ^= 0xFF;
		auto modifiedHash = TTraits::CalculateHash(*pEntity, generationHashSeed);

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
		EXPECT_EQ(utils::ParseByteArray<Hash256>(test::Deterministic_Block_Hash_String), hash);
	}

	TEST(TEST_CLASS, BlockHashDoesNotChangeWhenBlockTransactionDataChanges) {
		// Arrange:
		auto pBlock = BlockNormalTraits::Generate();
		auto originalHash = CalculateHash(*pBlock);

		// Act: change a transaction deadline
		//     (notice that in a properly constructed block, this change will cause the TransactionsHash to change
		//      in this test, that field is not set so the before and after hashes are equal)
		pBlock->TransactionsPtr()->Deadline = pBlock->TransactionsPtr()->Deadline + Timestamp(1);
		auto modifiedHash = CalculateHash(*pBlock);

		// Assert:
		EXPECT_EQ(originalHash, modifiedHash);
	}

	TEST(TEST_CLASS, BlockHashDoesNotChangeWhenBlockFooterChanges_Normal) {
		// Arrange:
		auto pBlock = BlockNormalTraits::Generate();
		auto originalHash = CalculateHash(*pBlock);

		// Act:
		GetBlockFooter<PaddedBlockFooter>(*pBlock).BlockHeader_Reserved1 ^= 0xFFFFFFFF;
		auto modifiedHash = CalculateHash(*pBlock);

		// Assert:
		EXPECT_EQ(originalHash, modifiedHash);
	}

	TEST(TEST_CLASS, BlockHashChangesWhenBlockFooterChanges_Importance) {
		// Arrange:
		auto pBlock = BlockImportanceTraits::Generate();
		auto originalHash = CalculateHash(*pBlock);

		// Act:
		GetBlockFooter<ImportanceBlockFooter>(*pBlock).PreviousImportanceBlockHash[0] ^= 0xFF;
		auto modifiedHash = CalculateHash(*pBlock);

		// Assert:
		EXPECT_NE(originalHash, modifiedHash);
	}

	// endregion

	// region CalculateHash - transaction

	TEST(TEST_CLASS, CalculateTransactionHashReturnsExpectedHash) {
		// Arrange: create a predefined transaction
		auto pTransaction = test::GenerateDeterministicTransaction();
		auto generationHashSeed = utils::ParseByteArray<GenerationHashSeed>(test::Deterministic_Network_Generation_Hash_Seed_String);

		// Act:
		auto hash = CalculateHash(*pTransaction, generationHashSeed);

		// Assert:
		EXPECT_EQ(utils::ParseByteArray<Hash256>(test::Deterministic_Transaction_Hash_String), hash);
	}

	TEST(TEST_CLASS, TransactionHashChangesWhenGenerationHashChanges) {
		// Arrange:
		auto pTransaction = TransactionTraits::Generate();
		auto originalHash = CalculateHash(*pTransaction, test::GenerateRandomByteArray<GenerationHashSeed>());

		// Act:
		auto modifiedHash = CalculateHash(*pTransaction, test::GenerateRandomByteArray<GenerationHashSeed>());

		// Assert:
		EXPECT_NE(originalHash, modifiedHash);
	}

	// endregion

	// region CalculateHash - transaction custom payload

	TEST(TEST_CLASS, TransactionCustomPayloadHashChangesWhenGenerationHashChanges) {
		// Arrange:
		auto pTransaction = TransactionCustomPayloadTraits::Generate();
		auto transactionBuffer = RawBuffer{ reinterpret_cast<uint8_t*>(pTransaction.get()), sizeof(Transaction) - 1 };
		auto originalHash = CalculateHash(*pTransaction, test::GenerateRandomByteArray<GenerationHashSeed>(), transactionBuffer);

		// Act:
		auto modifiedHash = CalculateHash(*pTransaction, test::GenerateRandomByteArray<GenerationHashSeed>(), transactionBuffer);

		// Assert:
		EXPECT_NE(originalHash, modifiedHash);
	}

	TEST(TEST_CLASS, TransactionCustomPayloadHashChangesWhenDataBufferDataChanges) {
		// Arrange:
		auto pTransaction = TransactionCustomPayloadTraits::Generate();
		const auto* pTransactionData = reinterpret_cast<uint8_t*>(pTransaction.get());
		auto generationHashSeed = test::GenerateRandomByteArray<GenerationHashSeed>();
		auto originalHash = CalculateHash(*pTransaction, generationHashSeed, { pTransactionData, sizeof(Transaction) - 1 });

		// Act:
		auto modifiedHash = CalculateHash(*pTransaction, generationHashSeed, { pTransactionData + 1, sizeof(Transaction) - 1 });

		// Assert:
		EXPECT_NE(originalHash, modifiedHash);
	}

	TEST(TEST_CLASS, TransactionCustomPayloadHashChangesWhenDataBufferSizeChanges) {
		// Arrange:
		auto pTransaction = TransactionCustomPayloadTraits::Generate();
		const auto* pTransactionData = reinterpret_cast<uint8_t*>(pTransaction.get());
		auto generationHashSeed = test::GenerateRandomByteArray<GenerationHashSeed>();
		auto originalHash = CalculateHash(*pTransaction, generationHashSeed, { pTransactionData, sizeof(Transaction) });

		// Act:
		auto modifiedHash = CalculateHash(*pTransaction, generationHashSeed, { pTransactionData, sizeof(Transaction) - 1 });

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
		auto transactionHash = test::GenerateRandomByteArray<Hash256>();

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
		auto transactionHash = test::GenerateRandomByteArray<Hash256>();

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

	// region CalculateMerkleTree (transaction element)

	namespace {
		auto CreateTransactionElements(const std::vector<std::shared_ptr<Transaction>>& transactions) {
			std::vector<TransactionElement> transactionElements;
			for (const auto& pTransaction : transactions) {
				transactionElements.emplace_back(*pTransaction);
				transactionElements.back().MerkleComponentHash = test::GenerateRandomByteArray<Hash256>();
			}

			return transactionElements;
		}

		void AssertMerkleTree(size_t numTransactions) {
			// Arrange:
			auto transactions = test::GenerateRandomTransactions(numTransactions);
			auto transactionElements = CreateTransactionElements(transactions);
			crypto::MerkleHashBuilder builder;
			for (const auto& transactionElement : transactionElements)
				builder.update(transactionElement.MerkleComponentHash);

			std::vector<Hash256> expectedMerkleTree;
			builder.final(expectedMerkleTree);

			// Act:
			auto merkleTree = CalculateMerkleTree(transactionElements);

			// Assert:
			ASSERT_EQ(expectedMerkleTree.size(), merkleTree.size());

			for (auto i = 0u; i < merkleTree.size(); ++i)
				EXPECT_EQ(expectedMerkleTree[i], merkleTree[i]) << "at index " << i;
		}
	}

	TEST(TEST_CLASS, CanCalculateMerkleTree_ZeroTransactionElements) {
		AssertMerkleTree(0);
	}

	TEST(TEST_CLASS, CanCalculateMerkleTree_SingleTransactionElement) {
		AssertMerkleTree(1);
	}

	TEST(TEST_CLASS, CanCalculateMerkleTree_MultipleTransactionElements) {
		AssertMerkleTree(5);
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
		auto generationHashSeed = test::GenerateRandomByteArray<GenerationHashSeed>();

		// - since there are no supplementary buffers, the transaction hash is equal to the merkle hash
		auto expectedEntityHash = CalculateHash(transaction, generationHashSeed, mocks::ExtractBuffer({ 5, 15 }, &transaction));

		// Act:
		UpdateHashes(registry, generationHashSeed, transactionElement);

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
		auto generationHashSeed = test::GenerateRandomByteArray<GenerationHashSeed>();

		auto expectedEntityHash = CalculateHash(transaction, generationHashSeed, mocks::ExtractBuffer({ 6, 10 }, &transaction));

		Hash256 expectedMerkleComponentHash;
		crypto::Sha3_256_Builder sha3;
		sha3.update(expectedEntityHash);
		sha3.update(mocks::ExtractBuffer({ 7, 11 }, &transaction));
		sha3.update(mocks::ExtractBuffer({ 4, 7 }, &transaction));
		sha3.update(mocks::ExtractBuffer({ 12, 20 }, &transaction));
		sha3.final(expectedMerkleComponentHash);

		// Act:
		UpdateHashes(registry, generationHashSeed, transactionElement);

		// Assert:
		EXPECT_EQ(expectedEntityHash, transactionElement.EntityHash);
		EXPECT_EQ(expectedMerkleComponentHash, transactionElement.MerkleComponentHash);
		EXPECT_NE(transactionElement.EntityHash, transactionElement.MerkleComponentHash);
	}

	// endregion
}}
