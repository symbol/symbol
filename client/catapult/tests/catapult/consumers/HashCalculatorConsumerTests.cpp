#include "catapult/consumers/BlockConsumers.h"
#include "sdk/src/extensions/BlockExtensions.h"
#include "catapult/consumers/TransactionConsumers.h"
#include "catapult/crypto/Hashes.h"
#include "catapult/model/EntityHasher.h"
#include "catapult/exceptions.h"
#include "tests/catapult/consumers/utils/ConsumerTestUtils.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/test/core/PacketTestUtils.h"
#include "tests/test/core/mocks/MockTransaction.h"
#include "tests/test/core/mocks/MockTransactionPluginWithCustomBuffers.h"
#include "tests/TestHarness.h"

using catapult::disruptor::ConsumerInput;

#define BLOCK_TEST_CLASS BlockHashCalculatorConsumerTests
#define TRANSACTION_TEST_CLASS TransactionHashCalculatorConsumerTests

namespace catapult { namespace consumers {

	// region BlockHashCalculatorConsumer

	namespace {
		constexpr uint32_t Transaction_Size = sizeof(mocks::MockTransaction);

		void WriteTransactionAt(std::vector<uint8_t>& buffer, size_t offset) {
			test::SetTransactionAt(buffer, offset, Transaction_Size);
			auto& tx = reinterpret_cast<model::Transaction&>(buffer[offset]);
			tx.Deadline = Timestamp(offset);
		}

		ConsumerInput CreateBlockConsumerInput(uint32_t numBlocks, uint32_t numTransactionsPerBlock) {
			uint32_t numBytesPerBlock = sizeof(model::Block) + numTransactionsPerBlock * Transaction_Size;
			std::vector<uint8_t> buffer(numBlocks * numBytesPerBlock);

			std::vector<size_t> offsets;
			for (auto i = 0u; i < numBlocks; ++i) {
				offsets.push_back(i * numBytesPerBlock);
				auto& block = reinterpret_cast<model::Block&>(buffer[offsets.back()]);
				block.Size = numBytesPerBlock;
				block.Type = model::EntityType::Block;

				for (auto j = 0u; j < numTransactionsPerBlock; ++j) {
					auto txOffset = offsets.back() + sizeof(model::Block) + j * Transaction_Size;
					WriteTransactionAt(buffer, txOffset);
				}

				extensions::UpdateBlockTransactionsHash(block);
			}

			auto range = model::BlockRange::CopyVariable(buffer.data(), buffer.size(), offsets);
			return ConsumerInput(std::move(range));
		}

		void AssertCorrectHash(const model::TransactionElement& element) {
			auto expectedEntityHash = model::CalculateHash(element.Transaction);
			EXPECT_EQ(expectedEntityHash, element.EntityHash);
			EXPECT_EQ(expectedEntityHash, element.MerkleComponentHash);
		}

		void AssertCorrectHashes(const model::BlockElement& element, uint32_t numExpectedTransactions) {
			auto numTransactions = 0u;
			for (const auto& txElement : element.Transactions) {
				AssertCorrectHash(txElement);
				++numTransactions;
			}

			EXPECT_EQ(numExpectedTransactions, numTransactions);
			EXPECT_EQ(model::CalculateHash(element.Block), element.EntityHash);
		}

		void AssertBlockHashesAreCalculatedCorrectly(uint32_t numBlocks, uint32_t numTransactionsPerBlock) {
			// Arrange:
			auto pRegistry = mocks::CreateDefaultTransactionRegistry();
			auto input = CreateBlockConsumerInput(numBlocks, numTransactionsPerBlock);
			auto& blockElements = input.blocks();

			// Act:
			auto result = CreateBlockHashCalculatorConsumer(*pRegistry)(blockElements);

			// Assert:
			test::AssertContinued(result);
			EXPECT_EQ(numBlocks, blockElements.size());
			for (const auto& blockElement : blockElements)
				AssertCorrectHashes(blockElement, numTransactionsPerBlock);
		}
	}

	TEST(BLOCK_TEST_CLASS, CanProcessZeroEntities) {
		// Assert:
		auto pRegistry = mocks::CreateDefaultTransactionRegistry();
		test::AssertPassthroughForEmptyInput(CreateBlockHashCalculatorConsumer(*pRegistry));
	}

	TEST(BLOCK_TEST_CLASS, CanProcessSingleEntity) {
		// Assert:
		AssertBlockHashesAreCalculatedCorrectly(1, 0);
	}

	TEST(BLOCK_TEST_CLASS, CanProcessSingleEntityWithTransactions) {
		// Assert:
		AssertBlockHashesAreCalculatedCorrectly(1, 3);
	}

	TEST(BLOCK_TEST_CLASS, CanProcessMultipleEntities) {
		// Assert:
		AssertBlockHashesAreCalculatedCorrectly(3, 0);
	}

	TEST(BLOCK_TEST_CLASS, CanProcessMultipleEntitiesWithTransactions) {
		// Assert:
		AssertBlockHashesAreCalculatedCorrectly(3, 4);
	}

	TEST(BLOCK_TEST_CLASS, CalculatesCorrectHashForDeterministicEntity) {
		// Arrange:
		auto pRegistry = mocks::CreateDefaultTransactionRegistry();
		auto pEntity = test::GenerateDeterministicBlock();
		auto input = ConsumerInput(model::BlockRange::FromEntity(std::move(pEntity)));
		auto& blockElements = input.blocks();

		// Act:
		auto result = CreateBlockHashCalculatorConsumer(*pRegistry)(blockElements);

		// Assert:
		test::AssertContinued(result);
		ASSERT_EQ(1u, blockElements.size());
		EXPECT_EQ(test::Deterministic_Block_Hash_String, test::ToHexString(blockElements[0].EntityHash));
	}

	TEST(BLOCK_TEST_CLASS, ExceptionIsPropagatedIfMalformedTransactionIsProcessed) {
		// Arrange: make the size of the third transaction invalid
		auto pRegistry = mocks::CreateDefaultTransactionRegistry();
		auto input = CreateBlockConsumerInput(3, 4);
		auto& blockElements = input.blocks();

		const auto* pTransaction = reinterpret_cast<const mocks::MockTransaction*>(&blockElements[1].Block + 1) + 2;
		const_cast<mocks::MockTransaction*>(pTransaction)->Size = 2 * sizeof(mocks::MockTransaction) + 1;

		// Act: transaction iteration throws an exception
		EXPECT_THROW(CreateBlockHashCalculatorConsumer(*pRegistry)(blockElements), catapult_runtime_error);
	}

	// region block transactions hash validation

	namespace {
		void AssertBlockWithMismatchedBlockTransactionsHashIsSkipped(
				uint32_t numBlocks,
				uint32_t numTransactionsPerBlock,
				uint32_t mismatchedIndex) {
			// Arrange: corrupt the block transactions hash
			auto pRegistry = mocks::CreateDefaultTransactionRegistry();
			auto input = CreateBlockConsumerInput(numBlocks, numTransactionsPerBlock);
			auto& blockElements = input.blocks();
			const_cast<model::Block&>(blockElements[mismatchedIndex].Block).BlockTransactionsHash[0] ^= 0xFF;

			// Act:
			auto result = CreateBlockHashCalculatorConsumer(*pRegistry)(blockElements);

			// Assert: the elements were skipped because a block transactions hash didn't match
			test::AssertAborted(result, Failure_Consumer_Block_Transactions_Hash_Mismatch);
		}
	}

	TEST(BLOCK_TEST_CLASS, SingleEntityIsSkippedIfBlockTransactionsHashDoesNotMatch) {
		// Assert:
		AssertBlockWithMismatchedBlockTransactionsHashIsSkipped(1, 0, 0);
		AssertBlockWithMismatchedBlockTransactionsHashIsSkipped(1, 3, 0);
	}

	TEST(BLOCK_TEST_CLASS, MultipleEntitiesAreSkippedIfAnyBlockTransactionsHashDoesNotMatch) {
		// Assert:
		AssertBlockWithMismatchedBlockTransactionsHashIsSkipped(3, 0, 1);
		AssertBlockWithMismatchedBlockTransactionsHashIsSkipped(3, 4, 1);
	}

	// endregion

	// endregion

	// region TransactionHashCalculatorConsumer

	namespace {
		ConsumerInput CreateTransactionConsumerInput(uint32_t numTransactions) {
			std::vector<uint8_t> buffer(numTransactions * Transaction_Size);

			std::vector<size_t> offsets;
			for (auto i = 0u; i < numTransactions; ++i) {
				auto txOffset = i * Transaction_Size;
				offsets.push_back(txOffset);
				WriteTransactionAt(buffer, txOffset);
			}

			auto range = model::TransactionRange::CopyVariable(buffer.data(), buffer.size(), offsets);
			return ConsumerInput(std::move(range));
		}

		void AssertTransactionHashesAreCalculatedCorrectly(uint32_t numTransactions) {
			// Arrange:
			auto pRegistry = mocks::CreateDefaultTransactionRegistry();
			auto input = CreateTransactionConsumerInput(numTransactions);
			auto& txElements = input.transactions();

			// Act:
			auto result = CreateTransactionHashCalculatorConsumer(*pRegistry)(txElements);

			// Assert:
			test::AssertContinued(result);
			EXPECT_EQ(numTransactions, txElements.size());
			for (const auto& txElement : txElements)
				AssertCorrectHash(txElement);
		}
	}

	TEST(TRANSACTION_TEST_CLASS, CanProcessZeroEntities) {
		// Assert:
		auto pRegistry = mocks::CreateDefaultTransactionRegistry();
		test::AssertPassthroughForEmptyInput(CreateTransactionHashCalculatorConsumer(*pRegistry));
	}

	TEST(TRANSACTION_TEST_CLASS, CanProcessSingleEntity) {
		// Assert:
		AssertTransactionHashesAreCalculatedCorrectly(1);
	}

	TEST(TRANSACTION_TEST_CLASS, CanProcessMultipleEntities) {
		// Assert:
		AssertTransactionHashesAreCalculatedCorrectly(3);
	}

	TEST(TRANSACTION_TEST_CLASS, CalculatesCorrectHashForDeterministicEntity) {
		// Arrange:
		auto pRegistry = mocks::CreateDefaultTransactionRegistry();
		auto pEntity = test::GenerateDeterministicTransaction();
		auto input = ConsumerInput(model::TransactionRange::FromEntity(std::move(pEntity)));
		auto& txElements = input.transactions();

		// Act:
		auto result = CreateTransactionHashCalculatorConsumer(*pRegistry)(txElements);

		// Assert:
		test::AssertContinued(result);
		ASSERT_EQ(1u, txElements.size());
		EXPECT_EQ(test::Deterministic_Transaction_Hash_String, test::ToHexString(txElements[0].EntityHash));
	}

	// endregion

	// region dependent hash calculation

	namespace {
		struct BlockTraits {
				static ConsumerInput CreateInput() {
					return CreateBlockConsumerInput(1, 1);
				}

				static void UpdateWithExpectedMerkleHash(ConsumerInput& input, const Hash256& merkleHash) {
					// set BlockTransactionsHash so that the consumer completes successfully
					const_cast<Hash256&>(input.blocks()[0].Block.BlockTransactionsHash) = merkleHash;
				}

				static auto Consume(const model::TransactionRegistry& registry, ConsumerInput& input) {
					return CreateBlockHashCalculatorConsumer(registry)(input.blocks());
				}

				static const auto& GetTransaction(const ConsumerInput& input) {
					return *input.blocks()[0].Block.TransactionsPtr();
				}

				static const auto& GetTransactionElement(const ConsumerInput& input) {
					return input.blocks()[0].Transactions[0];
				}
		};

		struct TransactionTraits {
				static ConsumerInput CreateInput() {
					return CreateTransactionConsumerInput(1);
				}

				static void UpdateWithExpectedMerkleHash(ConsumerInput&, const Hash256&) {
				}

				static auto Consume(const model::TransactionRegistry& registry, ConsumerInput& input) {
					return CreateTransactionHashCalculatorConsumer(registry)(input.transactions());
				}

				static const auto& GetTransaction(const ConsumerInput& input) {
					return input.transactions()[0].Transaction;
				}

				static const auto& GetTransactionElement(const ConsumerInput& input) {
					return input.transactions()[0];
				}
		};
	}

#define DEPENDENT_HASH_CALCULATION_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(BLOCK_TEST_CLASS, TEST_NAME) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<BlockTraits>(); } \
	TEST(TRANSACTION_TEST_CLASS, TEST_NAME) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<TransactionTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	DEPENDENT_HASH_CALCULATION_TEST(TransactionEntityHashIsDependentOnDataBuffer) {
		// Arrange:
		auto pPlugin = mocks::CreateMockTransactionPluginWithCustomBuffers(mocks::OffsetRange{ 5, 15 }, {});
		auto registry = model::TransactionRegistry();
		registry.registerPlugin(std::move(pPlugin));

		auto input = TTraits::CreateInput();
		const auto& transaction = TTraits::GetTransaction(input);

		// - since there are no supplementary buffers, the transaction hash is equal to the merkle hash
		auto expectedEntityHash = model::CalculateHash(transaction, mocks::ExtractBuffer({ 5, 15 }, &transaction));
		TTraits::UpdateWithExpectedMerkleHash(input, expectedEntityHash);

		// Act:
		auto result = TTraits::Consume(registry, input);

		// Assert:
		test::AssertContinued(result);

		const auto& txElement = TTraits::GetTransactionElement(input);
		EXPECT_EQ(expectedEntityHash, txElement.EntityHash);
		EXPECT_EQ(expectedEntityHash, txElement.MerkleComponentHash);
	}

	DEPENDENT_HASH_CALCULATION_TEST(TransactionMerkleComponentHashIsDependentOnMerkleSupplementaryBuffers) {
		// Arrange:
		auto pPlugin = mocks::CreateMockTransactionPluginWithCustomBuffers(
				mocks::OffsetRange{ 6, 10 },
				std::vector<mocks::OffsetRange>{ { 7, 11 }, { 4, 7 }, { 12, 20 } });
		auto registry = model::TransactionRegistry();
		registry.registerPlugin(std::move(pPlugin));

		auto input = TTraits::CreateInput();
		const auto& transaction = TTraits::GetTransaction(input);

		auto expectedEntityHash = model::CalculateHash(transaction, mocks::ExtractBuffer({ 6, 10 }, &transaction));

		Hash256 expectedMerkleComponentHash;
		crypto::Sha3_256_Builder sha3;
		sha3.update(expectedEntityHash);
		sha3.update(mocks::ExtractBuffer({ 7, 11 }, &transaction));
		sha3.update(mocks::ExtractBuffer({ 4, 7 }, &transaction));
		sha3.update(mocks::ExtractBuffer({ 12, 20 }, &transaction));
		sha3.final(expectedMerkleComponentHash);

		TTraits::UpdateWithExpectedMerkleHash(input, expectedMerkleComponentHash);

		// Act:
		auto result = TTraits::Consume(registry, input);

		// Assert:
		test::AssertContinued(result);

		const auto& txElement = TTraits::GetTransactionElement(input);
		EXPECT_EQ(expectedEntityHash, txElement.EntityHash);
		EXPECT_EQ(expectedMerkleComponentHash, txElement.MerkleComponentHash);
	}

	// endregion
}}
