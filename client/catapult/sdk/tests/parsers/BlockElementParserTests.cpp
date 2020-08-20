/**
*** Copyright (c) 2016-present,
*** Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp. All rights reserved.
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
			auto buffer = test::GenerateRandomVector(pBlock->Size + (1 + numTransactions) * 2 * Hash256::Size + bufferPadding);
			std::memcpy(buffer.data(), pBlock.get(), pBlock->Size);
			return buffer;
		}
	}

	TEST(TEST_CLASS, CannotParseBlockElementWhenReportedBlockSizeIsSmallerThanHeader) {
		// Arrange: invalidate the block size (too small)
		auto buffer = PrepareBlockElementBuffer(3);
		reinterpret_cast<model::Block&>(buffer[0]).Size = sizeof(model::BlockHeader) - 1;

		// Act + Assert:
		size_t numBytesConsumed;
		EXPECT_THROW(ParseBlockElement(buffer, numBytesConsumed), catapult_runtime_error);
	}

	TEST(TEST_CLASS, CannotParseBlockElementWhenHashesAreNotFullyPresent) {
		// Arrange: use a buffer one byte too small
		auto buffer = PrepareBlockElementBuffer(3);
		buffer.resize(buffer.size() - 1);

		// Act + Assert:
		size_t numBytesConsumed;
		EXPECT_THROW(ParseBlockElement(buffer, numBytesConsumed), catapult_runtime_error);
	}

	TEST(TEST_CLASS, CannotParseBlockElementWhenAnyHashIsMissing) {
		// Arrange: use a buffer missing a single hash
		auto buffer = PrepareBlockElementBuffer(3);
		buffer.resize(buffer.size() - Hash256::Size);

		// Act + Assert:
		size_t numBytesConsumed;
		EXPECT_THROW(ParseBlockElement(buffer, numBytesConsumed), catapult_runtime_error);
	}

	TEST(TEST_CLASS, CannotParseBlockElementWhenBlockHeaderDoesNotFit) {
		// Arrange: use a buffer one byte smaller than a block header
		auto buffer = PrepareBlockElementBuffer(3);
		buffer.resize(sizeof(model::BlockHeader) - 1);

		// Act + Assert:
		size_t numBytesConsumed;
		EXPECT_THROW(ParseBlockElement(buffer, numBytesConsumed), catapult_runtime_error);
	}

	TEST(TEST_CLASS, CannotParseBlockElementWhenReportedBlockSizeIsLargerThanRemainingData) {
		// Arrange: invalidate the block size (too big)
		auto buffer = PrepareBlockElementBuffer(3);
		buffer.resize(reinterpret_cast<model::Block&>(buffer[0]).Size - 1);

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
			const auto* pHash = reinterpret_cast<const Hash256*>(&buffer[pBlock->Size]);
			EXPECT_EQ(*pHash, blockElement.EntityHash);
			++pHash;
			EXPECT_EQ(reinterpret_cast<const GenerationHash&>(*pHash), blockElement.GenerationHash);
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
		AssertCanParseBlockElement(0, 0);
	}

	TEST(TEST_CLASS, CanParseBlockElementWithoutTransactionsWithPadding) {
		AssertCanParseBlockElement(0, 123);
	}

	TEST(TEST_CLASS, CanParseBlockElementWithTransactions) {
		AssertCanParseBlockElement(3, 0);
	}

	TEST(TEST_CLASS, CanParseBlockElementWithTransactionsWithPadding) {
		AssertCanParseBlockElement(3, 123);
	}

	// endregion
}}
