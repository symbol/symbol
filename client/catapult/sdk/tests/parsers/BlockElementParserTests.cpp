#include "src/parsers/BlockElementParser.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace parsers {

#define TEST_CLASS BlockElementParserTests

	// region failure

	namespace {
		std::vector<uint8_t> PrepareBlockElementBuffer(size_t numTransactions, size_t bufferPadding = 0) {
			// plus 1 because block metadata is composed of two hashes too
			auto pBlock = test::GenerateBlockWithTransactions(numTransactions);
			std::vector<uint8_t> buffer(pBlock->Size + (1 + numTransactions) * 2 * Hash256_Size + bufferPadding);
			test::FillWithRandomData(buffer);
			memcpy(buffer.data(), pBlock.get(), pBlock->Size);
			return buffer;
		}
	}

	TEST(TEST_CLASS, CannotParseBlockElementIfReportedBlockSizeIsSmallerThanHeader) {
		// Arrange: invalidate the block size (too small)
		auto buffer = PrepareBlockElementBuffer(3);
		reinterpret_cast<model::Block&>(*buffer.data()).Size = sizeof(model::Block) - 1;

		// Act + Assert:
		size_t numBytesConsumed;
		EXPECT_THROW(ParseBlockElement(buffer, numBytesConsumed), catapult_runtime_error);
	}

	TEST(TEST_CLASS, CannotParseBlockElementIfHashesAreNotFullyPresent) {
		// Arrange: use a buffer one byte too small
		auto buffer = PrepareBlockElementBuffer(3);
		buffer.resize(buffer.size() - 1);

		// Act + Assert:
		size_t numBytesConsumed;
		EXPECT_THROW(ParseBlockElement(buffer, numBytesConsumed), catapult_runtime_error);
	}

	TEST(TEST_CLASS, CannotParseBlockElementIfAnyHashIsMissing) {
		// Arrange: use a buffer missing a single hash
		auto buffer = PrepareBlockElementBuffer(3);
		buffer.resize(buffer.size() - Hash256_Size);

		// Act + Assert:
		size_t numBytesConsumed;
		EXPECT_THROW(ParseBlockElement(buffer, numBytesConsumed), catapult_runtime_error);
	}

	TEST(TEST_CLASS, CannotParseBlockElementIfBlockHeaderDoesNotFit) {
		// Arrange: use a buffer one byte smaller than a block header
		auto buffer = PrepareBlockElementBuffer(3);
		buffer.resize(sizeof(model::Block) - 1);

		// Act + Assert:
		size_t numBytesConsumed;
		EXPECT_THROW(ParseBlockElement(buffer, numBytesConsumed), catapult_runtime_error);
	}

	TEST(TEST_CLASS, CannotParseBlockElementIfReportedBlockSizeIsLargerThanRemainingData) {
		// Arrange: invalidate the block size (too big)
		auto buffer = PrepareBlockElementBuffer(3);
		buffer.resize(reinterpret_cast<model::Block&>(*buffer.data()).Size - 1);

		// Act + Assert:
		size_t numBytesConsumed;
		EXPECT_THROW(ParseBlockElement(buffer, numBytesConsumed), catapult_runtime_error);
	}

	// endregion

	// region success

	namespace {
		void AssertCanParseBlockElement(size_t numTransactions, size_t bufferPadding) {
			// Arrange:
			auto buffer = PrepareBlockElementBuffer(numTransactions, bufferPadding);
			const auto* pBlock = reinterpret_cast<const model::Block*>(buffer.data());

			// Act:
			size_t numBytesConsumed;
			auto blockElement = ParseBlockElement(buffer, numBytesConsumed);

			// Assert: compare block data
			EXPECT_EQ(buffer.size() - bufferPadding, numBytesConsumed);
			EXPECT_EQ(*pBlock, blockElement.Block);

			// - compare hashes
			const auto* pHash = reinterpret_cast<Hash256*>(&buffer[pBlock->Size]);
			EXPECT_EQ(*pHash, blockElement.EntityHash);
			++pHash;
			EXPECT_EQ(*pHash, blockElement.GenerationHash);
			++pHash;

			auto i = 0u;
			for (const auto& transactionElement : blockElement.Transactions) {
				auto message = "transaction at " + std::to_string(i++);
				EXPECT_EQ(*pHash, transactionElement.EntityHash) << message;
				++pHash;
				EXPECT_EQ(*pHash, transactionElement.MerkleComponentHash) << message;
				++pHash;
			}
		}
	}

	TEST(TEST_CLASS, CanParseBlockElementWithoutTransactions) {
		// Assert:
		AssertCanParseBlockElement(0, 0);
	}

	TEST(TEST_CLASS, CanParseBlockElementWithoutTransactionsWithPadding) {
		// Assert:
		AssertCanParseBlockElement(0, 123);
	}

	TEST(TEST_CLASS, CanParseBlockElementWithTransactions) {
		// Assert:
		AssertCanParseBlockElement(3, 0);
	}

	TEST(TEST_CLASS, CanParseBlockElementWithTransactionsWithPadding) {
		// Assert:
		AssertCanParseBlockElement(3, 123);
	}

	// endregion
}}
