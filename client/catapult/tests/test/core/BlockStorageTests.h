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
#include "BlockStatementTestUtils.h"
#include "BlockStorageTestUtils.h"
#include "BlockTestUtils.h"
#include "mocks/MockMemoryBlockStorage.h"
#include "mocks/MockMemoryStream.h"
#include "catapult/io/BlockStatementSerializer.h"
#include "catapult/model/BlockUtils.h"
#include "catapult/constants.h"
#include "tests/test/nodeps/Nemesis.h"
#include "tests/TestHarness.h"
#include <numeric>

namespace catapult { namespace test {

	// region LoadHashesTraits

	/// Load traits for verifying hashes.
	struct LoadHashesTraits {
		static auto Load(const io::BlockStorage& storage, Height height) {
			return storage.loadHashesFrom(height, 1);
		}

		static void Assert(const model::BlockElement& originalBlockElement, const model::HashRange& hashes) {
			// Assert:
			EXPECT_EQ(1u, hashes.size());
			EXPECT_EQ(originalBlockElement.EntityHash, *hashes.cbegin());
		}

		static void AssertLoadError(const io::BlockStorage& storage, Height height) {
			// Assert:
			auto result = Load(storage, height);
			EXPECT_EQ(0u, result.size());
		}
	};

	// endregion

	// region LoadBlockTraits

	/// Load traits for verifying blocks.
	struct LoadBlockTraits {
		static auto Load(const io::BlockStorage& storage, Height height) {
			return storage.loadBlock(height);
		}

		static void Assert(const model::BlockElement& originalBlockElement, const std::shared_ptr<const model::Block>& pBlock) {
			EXPECT_EQ(originalBlockElement.Block.Signature, pBlock->Signature);
			EXPECT_EQ(originalBlockElement.Block, *pBlock);
		}

		static void AssertLoadError(const io::BlockStorage& storage, Height height) {
			// Act + Assert:
			EXPECT_THROW(Load(storage, height), catapult_invalid_argument);
		}
	};

	// endregion

	// region LoadBlockElementTraits

	/// Load traits for verifying block elements.
	struct LoadBlockElementTraits {
		static auto Load(const io::BlockStorage& storage, Height height) {
			// note: block is saved with block statement, so when loading load them as well
			auto pBlockElement = storage.loadBlockElement(height);
			auto blockStatementPair = storage.loadBlockStatementData(height);

			// - deserialize block statement
			mocks::MockMemoryStream stream(blockStatementPair.first);
			auto pBlockStatement = std::make_shared<model::BlockStatement>();
			io::ReadBlockStatement(stream, *pBlockStatement);
			const_cast<model::BlockElement&>(*pBlockElement).OptionalStatement = std::move(pBlockStatement);
			return pBlockElement;
		}

		static void Assert(
				const model::BlockElement& originalBlockElement,
				const std::shared_ptr<const model::BlockElement>& pBlockElement) {
			// Assert:
			AssertEqual(originalBlockElement, *pBlockElement);
		}

		static void AssertLoadError(const io::BlockStorage& storage, Height height) {
			// Act + Assert:
			EXPECT_THROW(Load(storage, height), catapult_invalid_argument);
		}
	};

	// endregion

	// region LoadBlockStatementDataTraits

	/// Load traits for verifying block statement data.
	struct LoadBlockStatementDataTraits {
		static auto Load(const io::BlockStorage& storage, Height height) {
			return storage.loadBlockStatementData(height);
		}

		static void Assert(
				const model::BlockElement& originalBlockElement,
				const std::pair<std::vector<uint8_t>, bool>& blockElementPair) {
			// Assert:
			ASSERT_TRUE(!!originalBlockElement.OptionalStatement);
			auto expectedData = SerializeBlockStatement(*originalBlockElement.OptionalStatement);
			ASSERT_TRUE(blockElementPair.second);
			ASSERT_EQ(expectedData.size(), blockElementPair.first.size());
			EXPECT_EQ_MEMORY(expectedData.data(), blockElementPair.first.data(), expectedData.size());
		}

		static void AssertLoadError(const io::BlockStorage& storage, Height height) {
			// Act + Assert:
			EXPECT_THROW(Load(storage, height), catapult_invalid_argument);
		}
	};

	// endregion

	/// Block storage test suite.
	template<typename TTraits>
	struct BlockStorageTests {
	private:
		using StorageContext = StorageContextT<TTraits>;

	private:
		static auto PrepareStorageWithBlocks(size_t numBlocks) {
			return test::PrepareStorageWithBlocks<TTraits>(numBlocks);
		}

		static model::BlockElement BlockToBlockElementWithStatements(const model::Block& block, const Hash256& hash = {}) {
			auto blockElement = BlockToBlockElement(block, hash);
			blockElement.OptionalStatement = GenerateRandomStatements({ 2, 1, 3 });
			return blockElement;
		}

	public:
		// region chainHeight + saveBlock - success

		static void AssertSavingBlockWithHeightHigherThanChainHeightAltersChainHeight() {
			// Arrange:
			auto pStorage = PrepareStorageWithBlocks(10);

			// - save the initial block
			auto pNewBlock = GenerateBlockWithTransactions(5, Height(11));
			pStorage->saveBlock(BlockToBlockElementWithStatements(*pNewBlock));

			// Sanity:
			EXPECT_EQ(Height(11), pStorage->chainHeight());

			// Act:
			pNewBlock->Height = Height(12);
			pStorage->saveBlock(BlockToBlockElementWithStatements(*pNewBlock));

			// Assert:
			EXPECT_EQ(Height(12), pStorage->chainHeight());
		}

		static void AssertCanLoadNewlySavedBlock() {
			// Arrange:
			auto pStorage = PrepareStorageWithBlocks(10);

			auto pBlock = GenerateBlockWithTransactions(5, Height(11));
			auto expectedBlockElement = CreateBlockElementForSaveTests(*pBlock);

			// Act: save and load a block
			pStorage->saveBlock(expectedBlockElement);
			auto pBlockElement = pStorage->loadBlockElement(Height(11));

			// Assert:
			EXPECT_EQ(Height(11), pStorage->chainHeight());

			AssertEqual(expectedBlockElement, *pBlockElement);
		}

		static void AssertCanOverwriteBlockWithSameData() {
			// Arrange:
			auto pStorage = PrepareStorageWithBlocks(10);

			auto pBlock = GenerateBlockWithTransactions(5, Height(11));
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
			auto pStorage = PrepareStorageWithBlocks(10);

			auto pBlock = GenerateBlockWithTransactions(5, Height(11));
			pStorage->saveBlock(CreateBlockElementForSaveTests(*pBlock));

			auto expectedBlockElement = CreateBlockElementForSaveTests(*pBlock);

			// Act: drop, modify the block, save it
			pStorage->dropBlocksAfter(Height(10));
			pBlock->Timestamp = pBlock->Timestamp + Timestamp(1);

			pStorage->saveBlock(expectedBlockElement);
			auto pBlockElement = pStorage->loadBlockElement(Height(11));

			// Assert: the modified data was loaded
			EXPECT_EQ(Height(11), pStorage->chainHeight());

			AssertEqual(expectedBlockElement, *pBlockElement);
		}

		// endregion

		// region loadHashesFrom

	private:
		static void AssertNoHashes(uint32_t numBlocks, uint32_t maxHashes, Height requestHeight) {
			// Arrange:
			auto pStorage = PrepareStorageWithBlocks(numBlocks);

			// Act:
			auto hashes = pStorage->loadHashesFrom(requestHeight, maxHashes);

			// Assert:
			EXPECT_TRUE(hashes.empty());
		}

		static void AssertCanLoadHashes(
				size_t blocksCount,
				size_t maxHashesCount,
				Height startHeight,
				const std::vector<uint8_t>& expectedHashes) {
			// Arrange:
			auto pStorage = PrepareStorageWithBlocks(blocksCount);

			// Act:
			auto hashes = pStorage->loadHashesFrom(startHeight, maxHashesCount);

			// Assert:
			ASSERT_EQ(expectedHashes.size(), hashes.size());
			auto i = 0u;
			for (const auto& hash : hashes) {
				auto expectedHash = Hash256();
				expectedHash[Hash256::Size - 1] = expectedHashes[i++];
				EXPECT_EQ(expectedHash, hash);
			}
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

		static void AssertLoadHashesFrom_CanLoadSingleHash() {
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
			std::vector<uint8_t> expectedHashes;
			constexpr auto Start_Height = Files_Per_Storage_Directory - 5;
			for (auto i = static_cast<uint8_t>(Start_Height); expectedHashes.size() < 11; ++i)
				expectedHashes.push_back(i);

			StorageContext context;
			context.pTempDirectoryGuard = std::make_unique<typename TTraits::Guard>();
			context.pStorage = TTraits::PrepareStorage(context.pTempDirectoryGuard->name(), Height(Start_Height));
			SeedBlocks(*context.pStorage, Height(Start_Height), Height(Start_Height + 10));

			// Act:
			auto hashes = context.pStorage->loadHashesFrom(Height(Start_Height), 100);

			// Assert:
			ASSERT_EQ(expectedHashes.size(), hashes.size());
			auto i = 0u;
			for (const auto& hash : hashes) {
				auto expectedHash = Hash256();
				expectedHash[Hash256::Size - 1] = expectedHashes[i++];
				EXPECT_EQ(expectedHash, hash);
			}
		}

		// endregion

		// region saveBlock - statements

	private:
		static void AssertCanSaveBlockStatement(const std::vector<size_t>& numStatements) {
			// Arrange:
			auto pStorage = PrepareStorageWithBlocks(10);

			auto pBlock = GenerateBlockWithTransactions(5, Height(11));
			auto expectedBlockElement = CreateBlockElementForSaveTests(*pBlock);
			expectedBlockElement.OptionalStatement = GenerateRandomStatements(numStatements);

			// Act: save block with block statement and load saved block statement
			pStorage->saveBlock(expectedBlockElement);
			auto blockStatementPair = pStorage->loadBlockStatementData(Height(11));

			// Assert:
			EXPECT_EQ(Height(11), pStorage->chainHeight());

			auto expected = SerializeBlockStatement(*expectedBlockElement.OptionalStatement);
			ASSERT_TRUE(blockStatementPair.second);
			ASSERT_EQ(expected.size(), blockStatementPair.first.size());
			EXPECT_EQ_MEMORY(expected.data(), blockStatementPair.first.data(), expected.size());
		}

	public:
		static void AssertCanSaveBlockWithoutStatements() {
			AssertCanSaveBlockStatement({ 0, 0, 0 });
		}

		static void AssertCanSaveBlockWithOnlyTransactionStatements() {
			AssertCanSaveBlockStatement({ 5, 0, 0 });
		}

		static void AssertCanSaveBlockWithOnlyAddressResolutions() {
			AssertCanSaveBlockStatement({ 0, 8, 0 });
		}

		static void AssertCanSaveBlockWithOnlyMosaicResolutions() {
			AssertCanSaveBlockStatement({ 0, 0, 13 });
		}

		static void AssertCanSaveBlockWithAllStatements() {
			AssertCanSaveBlockStatement({ 5, 8, 13 });
		}

		// endregion

		// region saveBlock - failure
	private:
		static void AssertCannotSaveBlockAtHeight(size_t numSeedBlocks, Height newBlockHeight) {
			// Arrange:
			auto pStorage = PrepareStorageWithBlocks(numSeedBlocks);

			// Act + Assert:
			auto pNewBlock = GenerateBlockWithTransactions(5, newBlockHeight);
			EXPECT_THROW(pStorage->saveBlock(BlockToBlockElementWithStatements(*pNewBlock)), catapult_invalid_argument);
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

		// region dropBlocksAfter

		static void AssertCanDropBlocksAfterHeight() {
			// Arrange:
			auto pStorage = PrepareStorageWithBlocks(10);

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

		static void AssertCanDropBlocksAfterHeightAndSaveBlock() {
			// Arrange:
			auto pStorage = PrepareStorageWithBlocks(10);

			// Sanity:
			EXPECT_EQ(Height(10), pStorage->chainHeight());

			// Act:
			pStorage->dropBlocksAfter(Height(8));

			// - save the next block
			auto pNewBlock = GenerateBlockWithTransactions(5, Height(9));
			pStorage->saveBlock(CreateBlockElementForSaveTests(*pNewBlock));

			// Assert:
			EXPECT_EQ(Height(9), pStorage->chainHeight());

			EXPECT_TRUE(!!pStorage->loadBlockElement(Height(9)));
			EXPECT_THROW(pStorage->loadBlockElement(Height(10)), catapult_invalid_argument);
		}

		static void AssertCanDropAllBlocks() {
			// Arrange:
			auto pStorage = PrepareStorageWithBlocks(10);

			// Sanity:
			EXPECT_EQ(Height(10), pStorage->chainHeight());

			// Act:
			pStorage->dropBlocksAfter(Height(0));

			// Assert:
			EXPECT_EQ(Height(0), pStorage->chainHeight());

			EXPECT_THROW(pStorage->loadBlockElement(Height(1)), catapult_invalid_argument);
		}

		// endregion

		// region loadBlockElement - nemesis

		static void AssertStorageSeedInitiallyContainsNemesisBlock() {
			// Arrange:
			constexpr auto Source_Directory = "../seed/private-test";

			auto nemesisBlockElement = BlockToBlockElement(GetNemesisBlock(), GetNemesisGenerationHashSeed());

			// Act:
			const auto pStorage = TTraits::OpenStorage(Source_Directory);
			auto pBlockElement = pStorage->loadBlockElement(Height(1));

			// Assert:
			EXPECT_EQ(Height(1), pStorage->chainHeight());

			AssertEqual(nemesisBlockElement, *pBlockElement);
			EXPECT_TRUE(model::VerifyBlockHeaderSignature(pBlockElement->Block));
		}

		// endregion

		// region loading

		template<typename TLoadTraits>
		static void AssertCanLoadAtHeightLessThanChainHeight() {
			// Arrange:
			auto pStorage = PrepareStorageWithBlocks(10);

			auto pBlock = GenerateBlockWithTransactions(5, Height(11));
			auto blockElement = BlockToBlockElementWithStatements(*pBlock, GenerateRandomByteArray<Hash256>());
			pStorage->saveBlock(blockElement);
			auto pNextBlock = GenerateBlockWithTransactions(5, Height(12));
			auto nextBlockElement = BlockToBlockElementWithStatements(*pNextBlock);
			pStorage->saveBlock(nextBlockElement);

			// Act:
			auto result = TLoadTraits::Load(*pStorage, Height(11));

			// Assert:
			EXPECT_EQ(Height(12), pStorage->chainHeight());

			TLoadTraits::Assert(blockElement, result);
		}

		template<typename TLoadTraits>
		static void AssertCanLoadAtChainHeight() {
			// Arrange:
			auto pStorage = PrepareStorageWithBlocks(10);

			auto pBlock = GenerateBlockWithTransactions(5, Height(11));
			auto blockElement = BlockToBlockElementWithStatements(*pBlock, GenerateRandomByteArray<Hash256>());
			pStorage->saveBlock(blockElement);

			// Act:
			auto result = TLoadTraits::Load(*pStorage, Height(11));

			// Assert:
			EXPECT_EQ(Height(11), pStorage->chainHeight());

			TLoadTraits::Assert(blockElement, result);
		}

		template<typename TLoadTraits>
		static void AssertCannotLoadAtHeightGreaterThanChainHeight() {
			// Arrange:
			auto pStorage = PrepareStorageWithBlocks(10);

			// Act + Assert:
			TLoadTraits::AssertLoadError(*pStorage, Height(11));
		}

		template<typename TLoadTraits>
		static void AssertCanLoadMultipleSaved() {
			// Arrange:
			auto pStorage = PrepareStorageWithBlocks(5);

			auto pBlock1 = GenerateBlockWithTransactions(5, Height(6));
			auto pBlock2 = GenerateBlockWithTransactions(5, Height(7));
			auto blockElement1 = BlockToBlockElementWithStatements(*pBlock1, GenerateRandomByteArray<Hash256>());
			auto blockElement2 = BlockToBlockElementWithStatements(*pBlock2, GenerateRandomByteArray<Hash256>());
			pStorage->saveBlock(blockElement1);
			pStorage->saveBlock(blockElement2);

			// Act:
			auto result1 = TLoadTraits::Load(*pStorage, Height(6));
			auto result2 = TLoadTraits::Load(*pStorage, Height(7));

			// Assert:
			EXPECT_EQ(Height(7), pStorage->chainHeight());

			TLoadTraits::Assert(blockElement1, result1);
			TLoadTraits::Assert(blockElement2, result2);
		}

		// endregion

		// region purge

		static void AssertPurgeDestroysStorage() {
			// Arrange:
			auto pStorage = PrepareStorageWithBlocks(5);

			// Act:
			pStorage->purge();

			// Assert:
			EXPECT_EQ(Height(0), pStorage->chainHeight());

			EXPECT_THROW(pStorage->loadBlock(Height(1)), catapult_invalid_argument);
			EXPECT_THROW(pStorage->loadBlockElement(Height(1)), catapult_invalid_argument);
		}

		static void AssertCanSetHeightAfterPurge() {
			// Arrange:
			auto pStorage = PrepareStorageWithBlocks(5);

			// Act:
			pStorage->purge();
			pStorage->dropBlocksAfter(Height(7));

			// Assert:
			EXPECT_EQ(Height(7), pStorage->chainHeight());
		}

		// endregion
	};

// region MAKE/DEFINE TESTs

#define MAKE_BLOCK_STORAGE_LOAD_TEST(TRAITS_NAME, TEST_NAME, LOAD_TYPE) \
	TEST(TEST_CLASS, TEST_NAME##_##LOAD_TYPE) { \
		test::BlockStorageTests<TRAITS_NAME>::Assert##TEST_NAME<test::Load##LOAD_TYPE##Traits>(); \
	}

#define DEFINE_BLOCK_STORAGE_LOAD_TESTS(TRAITS_NAME, TEST_NAME) \
	MAKE_BLOCK_STORAGE_LOAD_TEST(TRAITS_NAME, TEST_NAME, Hashes) \
	MAKE_BLOCK_STORAGE_LOAD_TEST(TRAITS_NAME, TEST_NAME, Block) \
	MAKE_BLOCK_STORAGE_LOAD_TEST(TRAITS_NAME, TEST_NAME, BlockElement) \
	MAKE_BLOCK_STORAGE_LOAD_TEST(TRAITS_NAME, TEST_NAME, BlockStatementData)

#define MAKE_BLOCK_STORAGE_TEST(TRAITS_NAME, TEST_NAME) \
	TEST(TEST_CLASS, TEST_NAME) { test::BlockStorageTests<TRAITS_NAME>::Assert##TEST_NAME(); }

#define DEFINE_BLOCK_STORAGE_TESTS(TRAITS_NAME) \
	MAKE_BLOCK_STORAGE_TEST(TRAITS_NAME, SavingBlockWithHeightHigherThanChainHeightAltersChainHeight) \
	MAKE_BLOCK_STORAGE_TEST(TRAITS_NAME, CanLoadNewlySavedBlock) \
	MAKE_BLOCK_STORAGE_TEST(TRAITS_NAME, CanOverwriteBlockWithSameData) \
	MAKE_BLOCK_STORAGE_TEST(TRAITS_NAME, CanOverwriteBlockWithDifferentData) \
	\
	MAKE_BLOCK_STORAGE_TEST(TRAITS_NAME, LoadHashesFrom_LoadsZeroHashesWhenRequestHeightIsZero) \
	MAKE_BLOCK_STORAGE_TEST(TRAITS_NAME, LoadHashesFrom_LoadsZeroHashesWhenRequestHeightIsLargerThanLocalHeight) \
	MAKE_BLOCK_STORAGE_TEST(TRAITS_NAME, LoadHashesFrom_CanLoadSingleHash) \
	MAKE_BLOCK_STORAGE_TEST(TRAITS_NAME, LoadHashesFrom_CanLoadLastHash) \
	MAKE_BLOCK_STORAGE_TEST(TRAITS_NAME, LoadHashesFrom_LoadsAtMostMaxHashes) \
	MAKE_BLOCK_STORAGE_TEST(TRAITS_NAME, LoadHashesFrom_LoadsAreBoundedByLastBlock) \
	MAKE_BLOCK_STORAGE_TEST(TRAITS_NAME, LoadHashesFrom_LoadsCanCrossIndexFileBoundary) \
	\
	MAKE_BLOCK_STORAGE_TEST(TRAITS_NAME, CanSaveBlockWithoutStatements) \
	MAKE_BLOCK_STORAGE_TEST(TRAITS_NAME, CanSaveBlockWithOnlyTransactionStatements) \
	MAKE_BLOCK_STORAGE_TEST(TRAITS_NAME, CanSaveBlockWithOnlyAddressResolutions) \
	MAKE_BLOCK_STORAGE_TEST(TRAITS_NAME, CanSaveBlockWithOnlyMosaicResolutions) \
	MAKE_BLOCK_STORAGE_TEST(TRAITS_NAME, CanSaveBlockWithAllStatements) \
	\
	MAKE_BLOCK_STORAGE_TEST(TRAITS_NAME, CannotSaveBlockWithHeightLessThanChainHeight) \
	MAKE_BLOCK_STORAGE_TEST(TRAITS_NAME, CannotSaveBlockAtChainHeight) \
	MAKE_BLOCK_STORAGE_TEST(TRAITS_NAME, CannotSaveBlockMoreThanOneHeightBeyondChainHeight) \
	\
	MAKE_BLOCK_STORAGE_TEST(TRAITS_NAME, CanDropBlocksAfterHeight) \
	MAKE_BLOCK_STORAGE_TEST(TRAITS_NAME, CanDropBlocksAfterHeightAndSaveBlock) \
	MAKE_BLOCK_STORAGE_TEST(TRAITS_NAME, CanDropAllBlocks) \
	\
	MAKE_BLOCK_STORAGE_TEST(TRAITS_NAME, StorageSeedInitiallyContainsNemesisBlock) \
	\
	DEFINE_BLOCK_STORAGE_LOAD_TESTS(TRAITS_NAME, CanLoadAtHeightLessThanChainHeight) \
	DEFINE_BLOCK_STORAGE_LOAD_TESTS(TRAITS_NAME, CanLoadAtChainHeight) \
	DEFINE_BLOCK_STORAGE_LOAD_TESTS(TRAITS_NAME, CannotLoadAtHeightGreaterThanChainHeight) \
	DEFINE_BLOCK_STORAGE_LOAD_TESTS(TRAITS_NAME, CanLoadMultipleSaved)

#define DEFINE_PRUNABLE_BLOCK_STORAGE_TESTS(TRAITS_NAME) \
	MAKE_BLOCK_STORAGE_TEST(TRAITS_NAME, PurgeDestroysStorage) \
	MAKE_BLOCK_STORAGE_TEST(TRAITS_NAME, CanSetHeightAfterPurge)

// endregion
}}
