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

#pragma once
#include "catapult/model/BlockUtils.h"
#include "catapult/utils/HexFormatter.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/test/core/TransactionTestUtils.h"
#include "tests/test/core/mocks/MockMemoryBasedStorage.h"
#include "tests/test/nodeps/Nemesis.h"
#include <numeric>

namespace catapult { namespace test {

	/// Seeds \a storage with blocks starting at \a startHeight ending at \a endHeight inclusive.
	void SeedBlocks(io::BlockStorage& storage, Height startHeight, Height endHeight);

	/// Seeds \a storage with \a numBlocks blocks (storage will contain blocks with heights 1 - numBlocks).
	void SeedBlocks(io::BlockStorage& storage, size_t numBlocks);

	/// Creates block element from \a block with a random transaction hash.
	model::BlockElement CreateBlockElementForSaveTests(const model::Block& block);

	/// Context for holding storage and optional storage guard.
	template<typename TTraits>
	struct StorageContext {
		std::unique_ptr<typename TTraits::Guard> pTempDirectoryGuard;
		std::unique_ptr<typename TTraits::StorageType> pStorage;

	public:
		typename TTraits::StorageType& operator*() {
			return *pStorage;
		}

		typename TTraits::StorageType* operator->() {
			return pStorage.get();
		}
	};

	/// Prepares storage context, seeding storage with \a numBlocks.
	template<typename TTraits>
	auto PrepareStorageWithBlocks(size_t numBlocks) {
		StorageContext<TTraits> context;
		context.pTempDirectoryGuard = std::make_unique<typename TTraits::Guard>();
		context.pStorage = TTraits::PrepareStorage(context.pTempDirectoryGuard->name());
		SeedBlocks(*context.pStorage, numBlocks);
		return context;
	}

	/// Load traits for verifying block elements.
	struct LoadBlockElementTraits {
		static auto Load(const io::BlockStorage& storage, Height height) {
			return storage.loadBlockElement(height);
		}

		static void Assert(
				const model::Block& block,
				const Hash256& expectedHash,
				const std::shared_ptr<const model::BlockElement>& pBlockElement) {
			// Assert:
			EXPECT_EQ(block.Signature, pBlockElement->Block.Signature);
			EXPECT_EQ(block, pBlockElement->Block);
			EXPECT_EQ(expectedHash, pBlockElement->EntityHash);
		}

		static void AssertLoadError(const io::BlockStorage& storage, Height height) {
			// Act + Assert:
			EXPECT_THROW(Load(storage, height), catapult_invalid_argument);
		}
	};

	/// Load traits for verifying blocks.
	struct LoadBlockTraits {
		static auto Load(const io::BlockStorage& storage, Height height) {
			return storage.loadBlock(height);
		}

		static void Assert(const model::Block& block, const Hash256&, const std::shared_ptr<const model::Block>& pBlock) {
			EXPECT_EQ(block.Signature, pBlock->Signature);
			EXPECT_EQ(block, *pBlock);
		}

		static void AssertLoadError(const io::BlockStorage& storage, Height height) {
			EXPECT_THROW(Load(storage, height), catapult_invalid_argument);
		}
	};

	/// Load traits for verifying hashes.
	struct LoadHashesTraits {
		static auto Load(const io::BlockStorage& storage, Height height) {
			return storage.loadHashesFrom(height, 1);
		}

		static void Assert(const model::Block&, const Hash256& expectedHash, const model::HashRange& hashes) {
			// Assert:
			EXPECT_EQ(1u, hashes.size());
			EXPECT_EQ(expectedHash, *hashes.cbegin());
		}

		static void AssertLoadError(const io::BlockStorage& storage, Height height) {
			// Assert:
			auto result = Load(storage, height);
			EXPECT_EQ(0u, result.size());
		}
	};

	/// Block storage test suite.
	template<typename TTraits>
	struct BlockStorageTests {
	public:
		static void AssertStorageSeedInitiallyContainsNemesisBlock() {
			// Arrange:
#ifdef SIGNATURE_SCHEME_NIS1
			constexpr auto Source_Directory = "../seed/mijin-test.nis1";
#else
			constexpr auto Source_Directory = "../seed/mijin-test";
#endif

			const auto* pNemesisBlock = reinterpret_cast<const model::Block*>(&mocks::MemoryBasedStorage_NemesisBlockData);
			auto nemesisBlockElement = BlockToBlockElement(*pNemesisBlock);
			nemesisBlockElement.GenerationHash = GetNemesisGenerationHash();

			// Act:
			const auto pStorage = TTraits::OpenStorage(Source_Directory);
			auto pBlockElement = pStorage->loadBlockElement(Height(1));

			// Assert:
			EXPECT_EQ(Height(1), pStorage->chainHeight());
			AssertEqual(nemesisBlockElement, *pBlockElement);
			EXPECT_TRUE(model::VerifyBlockHeaderSignature(pBlockElement->Block));
		}

		// region block saving - success

		static void AssertSavingBlockWithHeightHigherThanChainHeightAltersChainHeight() {
			// Arrange:
			auto pStorage = PrepareStorageWithBlocks<TTraits>(10);

			// - save the initial block
			auto pNewBlock = GenerateBlockWithTransactionsAtHeight(Height(11));
			pStorage->saveBlock(BlockToBlockElement(*pNewBlock));

			// Sanity:
			EXPECT_EQ(Height(11), pStorage->chainHeight());

			// Act:
			pNewBlock->Height = Height(12);
			pStorage->saveBlock(BlockToBlockElement(*pNewBlock));

			// Assert:
			EXPECT_EQ(Height(12), pStorage->chainHeight());
		}

		static void AssertCanOverwriteBlockWithSameData() {
			// Arrange:
			auto pStorage = PrepareStorageWithBlocks<TTraits>(10);

			auto pBlock = GenerateBlockWithTransactionsAtHeight(Height(11));
			auto expectedBlockElement = CreateBlockElementForSaveTests(*pBlock);
			pStorage->saveBlock(expectedBlockElement);

			// Act: drop and save the same block again
			pStorage->dropBlocksAfter(Height(10));
			pStorage->saveBlock(expectedBlockElement);
			auto pBlockElement = pStorage->loadBlockElement(Height(11));

			// Assert:
			EXPECT_EQ(Height(11), pStorage->chainHeight());
			AssertEqual(expectedBlockElement, *pBlockElement);
		}

		static void AssertCanOverwriteBlockWithDifferentData() {
			// Arrange:
			auto pStorage = PrepareStorageWithBlocks<TTraits>(10);

			auto pBlock = GenerateBlockWithTransactionsAtHeight(Height(11));
			pStorage->saveBlock(CreateBlockElementForSaveTests(*pBlock));

			auto expectedBlockElement = CreateBlockElementForSaveTests(*pBlock);

			// Act: drop, modify the block, and save it
			pStorage->dropBlocksAfter(Height(10));
			pBlock->Timestamp = pBlock->Timestamp + Timestamp(1);

			pStorage->saveBlock(expectedBlockElement);
			auto pBlockElement = pStorage->loadBlockElement(Height(11));

			// Assert: the modified data was loaded
			EXPECT_EQ(Height(11), pStorage->chainHeight());
			AssertEqual(expectedBlockElement, *pBlockElement);
		}

		// endregion

		// region block saving - failure
	private:
		static void AssertCannotSaveBlockAtHeight(size_t numSeedBlocks, Height newBlockHeight) {
			// Arrange:
			auto pStorage = PrepareStorageWithBlocks<TTraits>(numSeedBlocks);

			// Act + Assert:
			auto pNewBlock = GenerateBlockWithTransactionsAtHeight(newBlockHeight);
			EXPECT_THROW(pStorage->saveBlock(BlockToBlockElement(*pNewBlock)), catapult_invalid_argument);
		}

	public:
		static void AssertCannotSaveBlockWithHeightLessThanChainHeight() {
			// Assert:
			AssertCannotSaveBlockAtHeight(10, Height(1));
			AssertCannotSaveBlockAtHeight(10, Height(9));
		}

		static void AssertCannotSaveBlockAtChainHeight() {
			// Assert:
			AssertCannotSaveBlockAtHeight(10, Height(10));
		}

		static void AssertCannotSaveBlockMoreThanOneHeightBeyondChainHeight() {
			// Assert:
			AssertCannotSaveBlockAtHeight(10, Height(12));
			AssertCannotSaveBlockAtHeight(10, Height(110));
		}

		// endregion

		// region loading

		template<typename TLoadTraits>
		static void AssertCanLoadAtHeightLessThanChainHeight() {
			// Arrange:
			auto pStorage = PrepareStorageWithBlocks<TTraits>(10);

			auto pBlock = GenerateBlockWithTransactionsAtHeight(Height(11));
			auto expectedHash = GenerateRandomData<Hash256_Size>();
			pStorage->saveBlock(BlockToBlockElement(*pBlock, expectedHash));
			auto pNextBlock = GenerateBlockWithTransactionsAtHeight(Height(12));
			pStorage->saveBlock(BlockToBlockElement(*pNextBlock));

			// Act:
			auto result = TLoadTraits::Load(*pStorage, Height(11));

			// Assert:
			EXPECT_EQ(Height(12), pStorage->chainHeight());
			TLoadTraits::Assert(*pBlock, expectedHash, result);
		}

		template<typename TLoadTraits>
		static void AssertCanLoadAtChainHeight() {
			// Arrange:
			auto pStorage = PrepareStorageWithBlocks<TTraits>(10);

			auto pBlock = GenerateBlockWithTransactionsAtHeight(Height(11));
			auto expectedHash = GenerateRandomData<Hash256_Size>();
			pStorage->saveBlock(BlockToBlockElement(*pBlock, expectedHash));

			// Act:
			auto result = TLoadTraits::Load(*pStorage, Height(11));

			// Assert:
			EXPECT_EQ(Height(11), pStorage->chainHeight());
			TLoadTraits::Assert(*pBlock, expectedHash, result);
		}

		template<typename TLoadTraits>
		static void AssertCannotLoadAtHeightGreaterThanChainHeight() {
			// Arrange:
			auto pStorage = PrepareStorageWithBlocks<TTraits>(10);

			// Act + Assert:
			TLoadTraits::AssertLoadError(*pStorage, Height(11));
		}

		template<typename TLoadTraits>
		static void AssertCanLoadMultipleSaved() {
			// Arrange:
			auto pStorage = PrepareStorageWithBlocks<TTraits>(5);

			auto pBlock1 = GenerateBlockWithTransactionsAtHeight(Height(6));
			auto pBlock2 = GenerateBlockWithTransactionsAtHeight(Height(7));
			auto expectedHash1 = GenerateRandomData<Hash256_Size>();
			auto expectedHash2 = GenerateRandomData<Hash256_Size>();
			pStorage->saveBlock(BlockToBlockElement(*pBlock1, expectedHash1));
			pStorage->saveBlock(BlockToBlockElement(*pBlock2, expectedHash2));

			// Act:
			auto result1 = TLoadTraits::Load(*pStorage, Height(6));
			auto result2 = TLoadTraits::Load(*pStorage, Height(7));

			// Assert:
			EXPECT_EQ(Height(7), pStorage->chainHeight());
			TLoadTraits::Assert(*pBlock1, expectedHash1, result1);
			TLoadTraits::Assert(*pBlock2, expectedHash2, result2);
		}

		// endregion

		// region drop blocks

		static void AssertCanDropBlocksAfterHeight() {
			// Arrange:
			auto pStorage = PrepareStorageWithBlocks<TTraits>(10);

			// Sanity:
			EXPECT_EQ(Height(10), pStorage->chainHeight());

			// Act:
			pStorage->dropBlocksAfter(Height(8));

			// Assert:
			EXPECT_EQ(Height(8), pStorage->chainHeight());
			EXPECT_TRUE(!!pStorage->loadBlockElement(Height(7)));
			EXPECT_TRUE(!!pStorage->loadBlockElement(Height(8)));
			EXPECT_THROW(pStorage->loadBlockElement(Height(9)), catapult_invalid_argument);
			EXPECT_THROW(pStorage->loadBlockElement(Height(10)), catapult_invalid_argument);
		}

		// endregion

	private:
		static void AssertNoHashes(uint32_t numBlocks, uint32_t maxHashes, Height requestHeight) {
			// Arrange:
			auto pStorage = PrepareStorageWithBlocks<TTraits>(numBlocks);

			// Act:
			auto hashes = pStorage->loadHashesFrom(requestHeight, maxHashes);

			// Assert:
			EXPECT_TRUE(hashes.empty());
		}

	public:
		static void AssertLoadHashesFrom_LoadsZeroHashesWhenRequestHeightIsZero() {
			AssertNoHashes(10, 1, Height(0));
		}

		static void AssertLoadHashesFrom_LoadsZeroHashesWhenRequestHeightIsLargerThanLocalHeight() {
			AssertNoHashes(10, 1, Height(11));
			AssertNoHashes(10, 5, Height(23));
			AssertNoHashes(10, 5, Height(100));
		}

		// region load hashes

	private:
		static void AssertCanLoadHashes(
				size_t blocksCount,
				size_t maxHashesCount,
				Height startHeight,
				const std::vector<uint8_t>& expectedHashes) {
			// Arrange:
			auto pStorage = PrepareStorageWithBlocks<TTraits>(blocksCount);

			// Act:
			auto hashes = pStorage->loadHashesFrom(startHeight, maxHashesCount);

			// Assert:
			ASSERT_EQ(expectedHashes.size(), hashes.size());
			auto i = 0u;
			for (const auto& hash : hashes) {
				auto expectedHash = Hash256();
				expectedHash[Hash256_Size - 1] = expectedHashes[i++];
				EXPECT_EQ(expectedHash, hash);
			}
		}

	public:
		static void AssertLoadHashesFrom_CanLoadASingleHash() {
			AssertCanLoadHashes(10u, 1u, Height(5), { 5 });
		}

		static void AssertLoadHashesFrom_CanLoadLastHash() {
			AssertCanLoadHashes(10u, 1u, Height(10), { 10 });
			AssertCanLoadHashes(10u, 5u, Height(10), { 10 });
		}

		static void AssertLoadHashesFrom_LoadsAtMostMaxHashes() {
			AssertCanLoadHashes(10u, 6u, Height(2), { 2, 3, 4, 5, 6, 7 });
		}

		static void AssertLoadHashesFrom_LoadsAreBoundedByLastBlock() {
			AssertCanLoadHashes(10u, 10u, Height(5), { 5, 6, 7, 8, 9, 10 });
		}

		// running this test does not make any sense, as it needs to produce quite some blocks
		static void AssertLoadHashesFrom_LoadsCanCrossIndexFileBoundary() {
			// Arrange: (note hashes are set inside SeedBlocks)
			std::vector<uint8_t> expectedHashes(11);
			std::iota(expectedHashes.begin(), expectedHashes.end(), static_cast<uint8_t>(65530 % 0xFF));

			StorageContext<TTraits> context;
			context.pTempDirectoryGuard = std::make_unique<typename TTraits::Guard>();
			context.pStorage = TTraits::PrepareStorage(context.pTempDirectoryGuard->name(), Height(65530));
			SeedBlocks(*context.pStorage, Height(65530), Height(65540));

			// Act:
			auto hashes = context.pStorage->loadHashesFrom(Height(65530), 100);

			// Assert:
			ASSERT_EQ(expectedHashes.size(), hashes.size());
			auto i = 0u;
			for (const auto& hash : hashes) {
				auto expectedHash = Hash256();
				expectedHash[Hash256_Size - 1] = expectedHashes[i++];
				EXPECT_EQ(expectedHash, hash);
			}
		}

		// endregion
	};

#define MAKE_BLOCK_STORAGE_TEST(TRAITS_NAME, TEST_NAME) \
	TEST(TEST_CLASS, TEST_NAME) { test::BlockStorageTests<TRAITS_NAME>::Assert##TEST_NAME(); }

#define DEFINE_BLOCK_STORAGE_LOAD_TESTS(TRAITS_NAME, TEST_NAME) \
	TEST(TEST_CLASS, TEST_NAME##_Block) { \
		test::BlockStorageTests<TRAITS_NAME>::Assert##TEST_NAME<test::LoadBlockTraits>(); \
	} \
	TEST(TEST_CLASS, TEST_NAME##_BlockElement) { \
		test::BlockStorageTests<TRAITS_NAME>::Assert##TEST_NAME<test::LoadBlockElementTraits>(); \
	} \
	TEST(TEST_CLASS, TEST_NAME##_Hashes) { \
		test::BlockStorageTests<TRAITS_NAME>::Assert##TEST_NAME<test::LoadHashesTraits>(); \
	}

#define DEFINE_BLOCK_STORAGE_TESTS(TRAITS_NAME) \
	MAKE_BLOCK_STORAGE_TEST(TRAITS_NAME, StorageSeedInitiallyContainsNemesisBlock) \
	MAKE_BLOCK_STORAGE_TEST(TRAITS_NAME, SavingBlockWithHeightHigherThanChainHeightAltersChainHeight) \
	MAKE_BLOCK_STORAGE_TEST(TRAITS_NAME, CanOverwriteBlockWithSameData) \
	MAKE_BLOCK_STORAGE_TEST(TRAITS_NAME, CanOverwriteBlockWithDifferentData) \
	\
	MAKE_BLOCK_STORAGE_TEST(TRAITS_NAME, CannotSaveBlockWithHeightLessThanChainHeight) \
	MAKE_BLOCK_STORAGE_TEST(TRAITS_NAME, CannotSaveBlockAtChainHeight) \
	MAKE_BLOCK_STORAGE_TEST(TRAITS_NAME, CannotSaveBlockMoreThanOneHeightBeyondChainHeight) \
	\
	DEFINE_BLOCK_STORAGE_LOAD_TESTS(TRAITS_NAME, CanLoadAtHeightLessThanChainHeight) \
	DEFINE_BLOCK_STORAGE_LOAD_TESTS(TRAITS_NAME, CanLoadAtChainHeight) \
	DEFINE_BLOCK_STORAGE_LOAD_TESTS(TRAITS_NAME, CannotLoadAtHeightGreaterThanChainHeight) \
	DEFINE_BLOCK_STORAGE_LOAD_TESTS(TRAITS_NAME, CanLoadMultipleSaved) \
	\
	MAKE_BLOCK_STORAGE_TEST(TRAITS_NAME, CanDropBlocksAfterHeight) \
	\
	MAKE_BLOCK_STORAGE_TEST(TRAITS_NAME, LoadHashesFrom_LoadsZeroHashesWhenRequestHeightIsZero) \
	MAKE_BLOCK_STORAGE_TEST(TRAITS_NAME, LoadHashesFrom_LoadsZeroHashesWhenRequestHeightIsLargerThanLocalHeight) \
	\
	MAKE_BLOCK_STORAGE_TEST(TRAITS_NAME, LoadHashesFrom_CanLoadASingleHash) \
	MAKE_BLOCK_STORAGE_TEST(TRAITS_NAME, LoadHashesFrom_CanLoadLastHash) \
	MAKE_BLOCK_STORAGE_TEST(TRAITS_NAME, LoadHashesFrom_LoadsAtMostMaxHashes) \
	MAKE_BLOCK_STORAGE_TEST(TRAITS_NAME, LoadHashesFrom_LoadsAreBoundedByLastBlock) \
	MAKE_BLOCK_STORAGE_TEST(TRAITS_NAME, LoadHashesFrom_LoadsCanCrossIndexFileBoundary)
}}
