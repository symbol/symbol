#include "src/extensions/BlockExtensions.h"
#include "src/extensions/TransactionExtensions.h"
#include "catapult/crypto/Hashes.h"
#include "catapult/crypto/MerkleHashBuilder.h"
#include "catapult/model/BlockUtils.h"
#include "catapult/model/EntityHasher.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/test/core/mocks/MockTransaction.h"
#include "tests/test/core/mocks/MockTransactionPluginWithCustomBuffers.h"
#include "tests/TestHarness.h"

#define TEST_CLASS BlockExtensionsTests

namespace catapult { namespace extensions {

	namespace {
		constexpr auto Default_Num_Transactions = 5u;

		auto CreateValidBlock(const crypto::KeyPair& signer, size_t numTransactions = Default_Num_Transactions) {
			// generate transactions
			test::ConstTransactions transactions;
			for (auto i = 1u; i <= numTransactions; ++i) {
				auto pTransaction = test::GenerateRandomTransaction();
				transactions.push_back(std::move(pTransaction));
			}

			// create block
			auto pBlock = test::GenerateBlockWithTransactions(signer, transactions);
			test::SignBlock(signer, *pBlock);
			return pBlock;
		}

		auto CreateValidBlock(size_t numTransactions = Default_Num_Transactions) {
			return CreateValidBlock(test::GenerateKeyPair(), numTransactions);
		}
	}

	// region Block Transactions Hash Extensions

	namespace {
		auto GenerateRandomBlockWithTransactions() {
			return test::GenerateBlockWithTransactionsAtHeight(7, 7);
		}

		Hash256 CalculateExpectedBlockTransactionsHash(const model::Block& block) {
			// calculate the expected block transactions hash
			crypto::MerkleHashBuilder builder;
			for (const auto& transaction : block.Transactions())
				builder.update(model::CalculateHash(transaction));

			Hash256 expectedBlockTransactionsHash;
			builder.final(expectedBlockTransactionsHash);
			return expectedBlockTransactionsHash;
		}

		template<typename THashCalculator>
		void AssertBlockTransactionsHashCalculation(THashCalculator blockTransactionsHashCalculator) {
			// Arrange: generate a random block and calculate the expected block transactions hash
			auto pBlock = GenerateRandomBlockWithTransactions();
			pBlock->BlockTransactionsHash = Hash256();
			auto expectedBlockTransactionsHash = CalculateExpectedBlockTransactionsHash(*pBlock);

			// Act: calculate the actual block transactions hash
			auto blockTransactionsHash = blockTransactionsHashCalculator(*pBlock);

			// Assert: the actual hash is nonzero and matches expected hash
			EXPECT_NE(Hash256(), blockTransactionsHash);
			EXPECT_EQ(test::ToHexString(expectedBlockTransactionsHash), test::ToHexString(blockTransactionsHash));
		}
	}

	TEST(TEST_CLASS, CanUpdateBlockTransactionsHash) {
		// Assert:
		AssertBlockTransactionsHashCalculation([](auto& block) {
			// Act:
			UpdateBlockTransactionsHash(block);
			return block.BlockTransactionsHash;
		});
	}

	TEST(TEST_CLASS, CanCalculateBlockTransactionsHash) {
		// Assert:
		AssertBlockTransactionsHashCalculation([](auto& block) {
			// Act:
			Hash256 blockTransactionsHash;
			CalculateBlockTransactionsHash(block, blockTransactionsHash);

			// Sanity: the block wasn't modified
			EXPECT_EQ(Hash256(), block.BlockTransactionsHash);
			return blockTransactionsHash;
		});
	}

	// endregion

	// region [Sign|Verify]FullBlock

	// region SignFullBlock

	TEST(TEST_CLASS, SignFullBlockProducesVerifiableBlock) {
		// Arrange: create a block and clear the signature and transactions hash
		auto signer = test::GenerateKeyPair();
		auto pBlock = CreateValidBlock(signer);
		pBlock->Signature = {};
		pBlock->BlockTransactionsHash = {};

		// Sanity: the block does not verify
		EXPECT_NE(VerifyFullBlockResult::Success, VerifyFullBlock(*pBlock));

		// Act: sign the block and then verify it
		SignFullBlock(signer, *pBlock);
		auto result = VerifyFullBlock(*pBlock);

		// Assert: fields have been updated and it is verifiable
		EXPECT_NE(Signature{}, pBlock->Signature);
		EXPECT_NE(Hash256{}, pBlock->BlockTransactionsHash);
		EXPECT_EQ(VerifyFullBlockResult::Success, result);
	}

	// endregion

	// region VerifyFullBlock

	TEST(TEST_CLASS, VerifyFullBlockSucceedsWhenVerifyingValidBlock) {
		// Arrange:
		auto pBlock = CreateValidBlock();

		// Act:
		auto result = VerifyFullBlock(*pBlock);

		// Assert:
		EXPECT_EQ(VerifyFullBlockResult::Success, result);
	}

	TEST(TEST_CLASS, VerifyFullBlockFailsWhenBlockDataIsAltered) {
		// Arrange:
		auto pBlock = CreateValidBlock();
		pBlock->Timestamp = pBlock->Timestamp + Timestamp(1);

		// Act:
		auto result = VerifyFullBlock(*pBlock);

		// Assert:
		EXPECT_EQ(VerifyFullBlockResult::Invalid_Block_Signature, result);
	}

	TEST(TEST_CLASS, VerifyFullBlockFailsWhenTransactionDataIsAltered) {
		// Arrange:
		auto signer = test::GenerateKeyPair();
		auto pBlock = CreateValidBlock(signer);
		auto it = ++pBlock->Transactions().begin();
		auto& transaction = reinterpret_cast<model::Transaction&>(*it);
		transaction.Deadline = transaction.Deadline + Timestamp(1);
		test::SignBlock(signer, *pBlock); // fix block transactions hash and block signature

		// Act:
		auto result = VerifyFullBlock(*pBlock);

		// Assert:
		EXPECT_EQ(VerifyFullBlockResult::Invalid_Transaction_Signature, result);
	}

	TEST(TEST_CLASS, VerifyFullBlockFailsWhenBlockTransactionsHashIsAltered) {
		// Arrange:
		auto signer = test::GenerateKeyPair();
		auto pBlock = CreateValidBlock(signer);
		pBlock->BlockTransactionsHash[0] ^= 0xFF;
		model::SignBlockHeader(signer, *pBlock); // fix block signature

		// Act:
		auto result = VerifyFullBlock(*pBlock);

		// Assert:
		EXPECT_EQ(VerifyFullBlockResult::Invalid_Block_Transactions_Hash, result);
	}

	// endregion

	// region Deterministic Entity Sanity

	TEST(TEST_CLASS, DeterministicTransactionIsFullyVerifiable) {
		// Arrange:
		auto pTransaction = test::GenerateDeterministicTransaction();

		// Act:
		auto isVerified = VerifyTransactionSignature(*pTransaction);

		// Assert:
		EXPECT_TRUE(isVerified);
	}

	TEST(TEST_CLASS, DeterministicBlockIsFullyVerifiable) {
		// Arrange:
		auto pBlock = test::GenerateDeterministicBlock();

		// Act:
		auto result = VerifyFullBlock(*pBlock);

		// Assert:
		EXPECT_EQ(VerifyFullBlockResult::Success, result);
	}

	// endregion

	// endregion

	// region ConvertBlockToBlockElement

	// region basic

	namespace {
		struct ConvertBlockToBlockElementBasicTraits {
			static auto Convert(const model::Block& block, const Hash256& generationHash) {
				return ConvertBlockToBlockElement(block, generationHash);
			}
		};

		struct ConvertBlockToBlockElementRegistryTraits {
			static auto Convert(const model::Block& block, const Hash256& generationHash) {
				auto pRegistry = mocks::CreateDefaultTransactionRegistry();
				return ConvertBlockToBlockElement(block, generationHash, *pRegistry);
			}
		};
	}

#define BASIC_BLOCK_TO_BLOCK_ELEMENT_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_WithoutRegistry) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<ConvertBlockToBlockElementBasicTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_WithRegistry) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<ConvertBlockToBlockElementRegistryTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	namespace {
		template<typename TTraits>
		model::BlockElement AssertBlockToBlockElementConversionExcludingTransactions(const model::Block& block) {
			// Arrange:
			auto generationHash = test::GenerateRandomData<Hash256_Size>();

			// Act:
			auto element = TTraits::Convert(block, generationHash);

			// Assert: the block element refers to the original block
			EXPECT_EQ(block, element.Block);
			EXPECT_EQ(model::CalculateHash(block), element.EntityHash);
			EXPECT_EQ(generationHash, element.GenerationHash);
			return element;
		}
	}

	BASIC_BLOCK_TO_BLOCK_ELEMENT_TEST(CanConvertBlockToBlockElement_WithoutTransactions) {
		// Arrange: create a block with no transactions
		auto pBlock = CreateValidBlock(0);

		// Act + Assert:
		auto element = AssertBlockToBlockElementConversionExcludingTransactions<TTraits>(*pBlock);

		// - no transactions are present
		EXPECT_TRUE(element.Transactions.empty());
	}

	BASIC_BLOCK_TO_BLOCK_ELEMENT_TEST(CanConvertBlockToBlockElement_WithTransactions) {
		// Arrange: create a block with transactions
		constexpr auto Num_Transactions = 7u;
		auto pBlock = CreateValidBlock(Num_Transactions);

		// Act + Assert:
		auto element = AssertBlockToBlockElementConversionExcludingTransactions<TTraits>(*pBlock);

		// - all transactions refer to transactions in the original block
		auto i = 0u;
		ASSERT_EQ(Num_Transactions, element.Transactions.size());
		for (const auto& transaction : pBlock->Transactions()) {
			const auto message = "tx at " + std::to_string(i);
			const auto& txElement = element.Transactions[i];
			auto entityHash = model::CalculateHash(transaction);

			// - since there are no supplementary buffers, the transaction hash is equal to the merkle hash
			EXPECT_EQ(transaction, txElement.Transaction) << message;
			EXPECT_EQ(entityHash, txElement.EntityHash) << message;
			EXPECT_EQ(entityHash, txElement.MerkleComponentHash) << message;
			++i;
		}

		EXPECT_EQ(Num_Transactions, i);
	}

	// endregion

	// region registry dependent hashes

	TEST(TEST_CLASS, CanConvertBlockToBlockElement_TransactionEntityHashIsDependentOnDataBuffer) {
		// Arrange:
		auto pPlugin = mocks::CreateMockTransactionPluginWithCustomBuffers(mocks::OffsetRange{ 5, 15 }, {});
		auto registry = model::TransactionRegistry();
		registry.registerPlugin(std::move(pPlugin));

		auto pBlock = CreateValidBlock(1);
		const auto& transaction = *pBlock->Transactions().cbegin();

		// - since there are no supplementary buffers, the transaction hash is equal to the merkle hash
		auto expectedEntityHash = model::CalculateHash(transaction, mocks::ExtractBuffer({ 5, 15 }, &transaction));

		// Act:
		auto element = ConvertBlockToBlockElement(*pBlock, Hash256(), registry);

		// Assert:
		ASSERT_EQ(1u, element.Transactions.size());

		const auto& txElement = element.Transactions[0];
		EXPECT_EQ(expectedEntityHash, txElement.EntityHash);
		EXPECT_EQ(expectedEntityHash, txElement.MerkleComponentHash);
	}

	TEST(TEST_CLASS, CanConvertBlockToBlockElement_TransactionMerkleComponentHashIsDependentOnMerkleSupplementaryBuffers) {
		// Arrange:
		auto pPlugin = mocks::CreateMockTransactionPluginWithCustomBuffers(
				mocks::OffsetRange{ 6, 10 },
				std::vector<mocks::OffsetRange>{ { 7, 11 }, { 4, 7 }, { 12, 20 } });
		auto registry = model::TransactionRegistry();
		registry.registerPlugin(std::move(pPlugin));

		auto pBlock = CreateValidBlock(1);
		const auto& transaction = *pBlock->Transactions().cbegin();

		auto expectedEntityHash = model::CalculateHash(transaction, mocks::ExtractBuffer({ 6, 10 }, &transaction));

		Hash256 expectedMerkleComponentHash;
		crypto::Sha3_256_Builder sha3;
		sha3.update(expectedEntityHash);
		sha3.update(mocks::ExtractBuffer({ 7, 11 }, &transaction));
		sha3.update(mocks::ExtractBuffer({ 4, 7 }, &transaction));
		sha3.update(mocks::ExtractBuffer({ 12, 20 }, &transaction));
		sha3.final(expectedMerkleComponentHash);

		// Act:
		auto element = ConvertBlockToBlockElement(*pBlock, Hash256(), registry);

		// Assert:
		ASSERT_EQ(1u, element.Transactions.size());

		const auto& txElement = element.Transactions[0];
		EXPECT_EQ(expectedEntityHash, txElement.EntityHash);
		EXPECT_EQ(expectedMerkleComponentHash, txElement.MerkleComponentHash);
	}

	// endregion

	// endregion
}}
