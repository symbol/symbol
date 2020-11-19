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

#include "catapult/io/BlockElementSerializer.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/test/core/SerializerTestUtils.h"
#include "tests/test/core/mocks/MockMemoryStream.h"
#include "tests/TestHarness.h"

namespace catapult { namespace io {

#define TEST_CLASS BlockElementSerializerTests

	// region WriteBlockElement

	namespace {
		struct WriteTestContext {
			std::unique_ptr<model::Block> pBlock;
			std::vector<Hash256> Hashes;
			std::unique_ptr<model::BlockElement> pBlockElement;
		};

		WriteTestContext PrepareWriteTestContext(uint32_t numTransactions, uint32_t numSubCacheMerkleRoots) {
			WriteTestContext context;
			context.pBlock = test::GenerateBlockWithTransactions(numTransactions);
			context.Hashes = test::GenerateRandomDataVector<Hash256>(2 + 2 * numTransactions + numSubCacheMerkleRoots);

			context.pBlockElement = std::make_unique<model::BlockElement>(*context.pBlock);
			context.pBlockElement->EntityHash = context.Hashes[0];
			std::copy(context.Hashes[1].cbegin(), context.Hashes[1].cend(), context.pBlockElement->GenerationHash.begin());

			auto hashOffset = 2u;
			for (auto& transaction : context.pBlock->Transactions()) {
				context.pBlockElement->Transactions.emplace_back(transaction);
				context.pBlockElement->Transactions.back().EntityHash = context.Hashes[hashOffset];
				context.pBlockElement->Transactions.back().MerkleComponentHash = context.Hashes[hashOffset + 1];
				hashOffset += 2;
			}

			context.pBlockElement->SubCacheMerkleRoots = std::vector<Hash256>(
					&context.Hashes[hashOffset],
					&context.Hashes[context.Hashes.size()]);
			return context;
		}

		std::vector<Hash256> ExtractHashes(const std::vector<uint8_t>& buffer, size_t blockSize) {
			std::vector<Hash256> hashes;

			// 1. extract block element hashes
			const auto* pHash = reinterpret_cast<const Hash256*>(buffer.data() + blockSize);
			hashes.insert(hashes.end(), pHash, pHash + 2);

			// 2. extract transaction element hashes
			const auto* pData = buffer.data() + blockSize + hashes.size() * Hash256::Size;
			auto numTransactionHashes = reinterpret_cast<const uint32_t&>(*pData);
			pHash = reinterpret_cast<const Hash256*>(pData + sizeof(uint32_t));
			hashes.insert(hashes.end(), pHash, pHash + 2 * numTransactionHashes);

			// 3. extract sub cache merkle roots
			pData += sizeof(uint32_t) + 2 * numTransactionHashes * Hash256::Size;
			auto numSubCacheMerkleRoots = reinterpret_cast<const uint32_t&>(*pData);
			pHash = reinterpret_cast<const Hash256*>(pData + sizeof(uint32_t));
			hashes.insert(hashes.end(), pHash, pHash + numSubCacheMerkleRoots);

			return hashes;
		}
	}

	TEST(TEST_CLASS, CanWriteBlockElement) {
		// Arrange:
		auto context = PrepareWriteTestContext(0, 0);

		std::vector<uint8_t> buffer;
		mocks::MockMemoryStream outputStream(buffer);

		// Act:
		WriteBlockElement(*context.pBlockElement, outputStream);

		// Assert:
		ASSERT_EQ(context.pBlock->Size + 2 * Hash256::Size + 2 * sizeof(uint32_t), buffer.size());
		EXPECT_EQ(*context.pBlock, reinterpret_cast<const model::Block&>(buffer[0]));

		auto savedHashes = ExtractHashes(buffer, context.pBlock->Size);
		EXPECT_EQ(2u, savedHashes.size());
		EXPECT_EQ(context.Hashes, savedHashes);
	}

	TEST(TEST_CLASS, CanWriteBlockElementWithTransactionHashes) {
		// Arrange:
		auto context = PrepareWriteTestContext(3, 0);

		std::vector<uint8_t> buffer;
		mocks::MockMemoryStream outputStream(buffer);

		// Act:
		WriteBlockElement(*context.pBlockElement, outputStream);

		// Assert:
		ASSERT_EQ(context.pBlock->Size + 8 * Hash256::Size + 2 * sizeof(uint32_t), buffer.size());
		EXPECT_EQ(*context.pBlock, reinterpret_cast<const model::Block&>(buffer[0]));

		auto savedHashes = ExtractHashes(buffer, context.pBlock->Size);
		EXPECT_EQ(8u, savedHashes.size());
		EXPECT_EQ(context.Hashes, savedHashes);
	}

	TEST(TEST_CLASS, CanWriteBlockElementWithSubCacheMerkleRoots) {
		// Arrange:
		auto context = PrepareWriteTestContext(0, 4);

		std::vector<uint8_t> buffer;
		mocks::MockMemoryStream outputStream(buffer);

		// Act:
		WriteBlockElement(*context.pBlockElement, outputStream);

		// Assert:
		ASSERT_EQ(context.pBlock->Size + 6 * Hash256::Size + 2 * sizeof(uint32_t), buffer.size());
		EXPECT_EQ(*context.pBlock, reinterpret_cast<const model::Block&>(buffer[0]));

		auto savedHashes = ExtractHashes(buffer, context.pBlock->Size);
		EXPECT_EQ(6u, savedHashes.size());
		EXPECT_EQ(context.Hashes, savedHashes);
	}

	TEST(TEST_CLASS, CanWriteBlockElementWithTransactionHashesAndSubCacheMerkleRoots) {
		// Arrange:
		auto context = PrepareWriteTestContext(3, 4);

		std::vector<uint8_t> buffer;
		mocks::MockMemoryStream outputStream(buffer);

		// Act:
		WriteBlockElement(*context.pBlockElement, outputStream);

		// Assert:
		ASSERT_EQ(context.pBlock->Size + 12 * Hash256::Size + 2 * sizeof(uint32_t), buffer.size());
		EXPECT_EQ(*context.pBlock, reinterpret_cast<const model::Block&>(buffer[0]));

		auto savedHashes = ExtractHashes(buffer, context.pBlock->Size);
		EXPECT_EQ(12u, savedHashes.size());
		EXPECT_EQ(context.Hashes, savedHashes);
	}

	// endregion

	// region ReadBlockElement

	namespace {
		struct ReadTestContext {
			std::unique_ptr<model::Block> pBlock;
			std::vector<Hash256> Hashes;
			std::vector<uint8_t> Buffer;
			catapult::GenerationHash GenerationHash;
		};

		ReadTestContext PrepareReadTestContext(uint32_t numTransactions, uint32_t numSubCacheMerkleRoots) {
			ReadTestContext context;
			context.pBlock = test::GenerateBlockWithTransactions(numTransactions);
			context.Hashes = test::GenerateRandomDataVector<Hash256>(2 + 2 * numTransactions + numSubCacheMerkleRoots);
			context.Buffer.resize(context.pBlock->Size + context.Hashes.size() * Hash256::Size + 2 * sizeof(uint32_t));
			std::copy(context.Hashes[1].cbegin(), context.Hashes[1].cend(), context.GenerationHash.begin());

			const auto* pNextHash = context.Hashes.data();
			std::memcpy(&context.Buffer[0], context.pBlock.get(), context.pBlock->Size);
			std::memcpy(&context.Buffer[context.pBlock->Size], pNextHash, 2 * Hash256::Size);

			auto offset = context.pBlock->Size + 2 * Hash256::Size;
			pNextHash += 2;
			std::memcpy(&context.Buffer[offset], &numTransactions, sizeof(uint32_t));
			std::memcpy(&context.Buffer[offset + sizeof(uint32_t)], pNextHash, 2 * numTransactions * Hash256::Size);

			offset += sizeof(uint32_t) + 2 * numTransactions * Hash256::Size;
			pNextHash += 2 * numTransactions;
			std::memcpy(&context.Buffer[offset], &numSubCacheMerkleRoots, sizeof(uint32_t));
			std::memcpy(&context.Buffer[offset + sizeof(uint32_t)], pNextHash, numSubCacheMerkleRoots * Hash256::Size);
			return context;
		}

		void AssertReadTransactions(const ReadTestContext& context, const model::BlockElement& blockElement) {
			auto i = 0u;
			auto blockTransactions = context.pBlock->Transactions();
			auto blockTransactionsIter = blockTransactions.begin();
			for (const auto& transactionElement : blockElement.Transactions) {
				auto message = "transaction at " + std::to_string(i);
				EXPECT_EQ(*blockTransactionsIter, transactionElement.Transaction) << message;
				EXPECT_EQ(context.Hashes[2 + i * 2], transactionElement.EntityHash) << message;
				EXPECT_EQ(context.Hashes[2 + i * 2 + 1], transactionElement.MerkleComponentHash) << message;
				++blockTransactionsIter;
				++i;
			}
		}
	}

	TEST(TEST_CLASS, CanReadBlockElement) {
		// Arrange:
		auto context = PrepareReadTestContext(0, 0);
		mocks::MockMemoryStream inputStream(context.Buffer);

		// Sanity:
		ASSERT_EQ(context.pBlock->Size + 2 * Hash256::Size + 2 * sizeof(uint32_t), context.Buffer.size());

		// Act:
		auto pBlockElement = ReadBlockElement(inputStream);

		// Assert:
		EXPECT_EQ(*context.pBlock, pBlockElement->Block);
		EXPECT_EQ(context.Hashes[0], pBlockElement->EntityHash);
		EXPECT_EQ(context.GenerationHash, pBlockElement->GenerationHash);

		EXPECT_TRUE(pBlockElement->SubCacheMerkleRoots.empty());
		EXPECT_TRUE(pBlockElement->Transactions.empty());
		EXPECT_FALSE(!!pBlockElement->OptionalStatement);
	}

	TEST(TEST_CLASS, CanReadBlockElementWithTransactionHashes) {
		// Arrange:
		auto context = PrepareReadTestContext(3, 0);
		mocks::MockMemoryStream inputStream(context.Buffer);

		// Sanity:
		ASSERT_EQ(context.pBlock->Size + 8 * Hash256::Size + 2 * sizeof(uint32_t), context.Buffer.size());

		// Act:
		auto pBlockElement = ReadBlockElement(inputStream);

		// Assert:
		EXPECT_EQ(*context.pBlock, pBlockElement->Block);
		EXPECT_EQ(context.Hashes[0], pBlockElement->EntityHash);
		EXPECT_EQ(context.GenerationHash, pBlockElement->GenerationHash);

		EXPECT_TRUE(pBlockElement->SubCacheMerkleRoots.empty());
		ASSERT_EQ(3u, pBlockElement->Transactions.size());
		AssertReadTransactions(context, *pBlockElement);
		EXPECT_FALSE(!!pBlockElement->OptionalStatement);
	}

	TEST(TEST_CLASS, CanReadBlockElementWithSubCacheMerkleRoots) {
		// Arrange:
		auto context = PrepareReadTestContext(0, 4);
		mocks::MockMemoryStream inputStream(context.Buffer);

		// Sanity:
		ASSERT_EQ(context.pBlock->Size + 6 * Hash256::Size + 2 * sizeof(uint32_t), context.Buffer.size());

		// Act:
		auto pBlockElement = ReadBlockElement(inputStream);

		// Assert:
		EXPECT_EQ(*context.pBlock, pBlockElement->Block);
		EXPECT_EQ(context.Hashes[0], pBlockElement->EntityHash);
		EXPECT_EQ(context.GenerationHash, pBlockElement->GenerationHash);

		ASSERT_EQ(4u, pBlockElement->SubCacheMerkleRoots.size());
		EXPECT_EQ(std::vector<Hash256>(&context.Hashes[2], &context.Hashes[6]), pBlockElement->SubCacheMerkleRoots);
		EXPECT_TRUE(pBlockElement->Transactions.empty());
		EXPECT_FALSE(!!pBlockElement->OptionalStatement);
	}

	TEST(TEST_CLASS, CanReadBlockElementWithTransactionHashesAndSubCacheMerkleRoots) {
		// Arrange:
		auto context = PrepareReadTestContext(3, 4);
		mocks::MockMemoryStream inputStream(context.Buffer);

		// Sanity:
		ASSERT_EQ(context.pBlock->Size + 12 * Hash256::Size + 2 * sizeof(uint32_t), context.Buffer.size());

		// Act:
		auto pBlockElement = ReadBlockElement(inputStream);

		// Assert:
		EXPECT_EQ(*context.pBlock, pBlockElement->Block);
		EXPECT_EQ(context.Hashes[0], pBlockElement->EntityHash);
		EXPECT_EQ(context.GenerationHash, pBlockElement->GenerationHash);

		ASSERT_EQ(4u, pBlockElement->SubCacheMerkleRoots.size());
		EXPECT_EQ(std::vector<Hash256>(&context.Hashes[8], &context.Hashes[12]), pBlockElement->SubCacheMerkleRoots);
		ASSERT_EQ(3u, pBlockElement->Transactions.size());
		AssertReadTransactions(context, *pBlockElement);
		EXPECT_FALSE(!!pBlockElement->OptionalStatement);
	}

	// endregion

	// region Roundtrip

	namespace {
		void RunRoundtripTest(const model::BlockElement& originalBlockElement) {
			// Act:
			auto pResult = test::RunRoundtripBufferTest(originalBlockElement, WriteBlockElement, ReadBlockElement);

			// Assert:
			test::AssertEqual(originalBlockElement, *pResult);
		}
	}

	TEST(TEST_CLASS, CanRoundtripBlockElement) {
		// Arrange:
		auto context = PrepareWriteTestContext(0, 0);

		// Act + Assert:
		RunRoundtripTest(*context.pBlockElement);
	}

	TEST(TEST_CLASS, CanRoundtripBlockElementWithTransactionHashes) {
		// Arrange:
		auto context = PrepareWriteTestContext(3, 0);

		// Act + Assert:
		RunRoundtripTest(*context.pBlockElement);
	}

	TEST(TEST_CLASS, CanRoundtripBlockElementWithSubCacheMerkleRoots) {
		// Arrange:
		auto context = PrepareWriteTestContext(0, 4);

		// Act + Assert:
		RunRoundtripTest(*context.pBlockElement);
	}

	TEST(TEST_CLASS, CanRoundtripBlockElementWithTransactionHashesAndSubCacheMerkleRoots) {
		// Arrange:
		auto context = PrepareWriteTestContext(3, 4);

		// Act + Assert:
		RunRoundtripTest(*context.pBlockElement);
	}

	// endregion
}}
