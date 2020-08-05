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

#include "catapult/io/BlockStorageCache.h"
#include "tests/test/core/BlockStorageTests.h"
#include "tests/test/nodeps/LockTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace io {

#define TEST_CLASS BlockStorageCacheTests

	namespace {
		// region BlockStorageCacheToBlockStorageAdapter

		// wraps a BlockStorageCache in a BlockStorage so that it can be tested via the tests in BlockStorageTests.h
		class BlockStorageCacheToBlockStorageAdapter : public BlockStorage {
		public:
			explicit BlockStorageCacheToBlockStorageAdapter(std::unique_ptr<BlockStorage>&& pStorage)
					: m_cache(std::move(pStorage), mocks::CreateMemoryBlockStorage(0))
			{}

		public: // LightBlockStorage
			Height chainHeight() const override {
				return m_cache.view().chainHeight();
			}

			model::HashRange loadHashesFrom(Height height, size_t maxHashes) const override {
				return m_cache.view().loadHashesFrom(height, maxHashes);
			}

			void saveBlock(const model::BlockElement& blockElement) override {
				auto modifier = m_cache.modifier();
				modifier.saveBlock(blockElement);
				modifier.commit();
			}

			void dropBlocksAfter(Height height) override {
				auto modifier = m_cache.modifier();
				modifier.dropBlocksAfter(height);
				modifier.commit();
			}

		public: // BlockStorage
			std::shared_ptr<const model::Block> loadBlock(Height height) const override {
				return m_cache.view().loadBlock(height);
			}

			std::shared_ptr<const model::BlockElement> loadBlockElement(Height height) const override {
				return m_cache.view().loadBlockElement(height);
			}

			std::pair<std::vector<uint8_t>, bool> loadBlockStatementData(Height height) const override {
				return m_cache.view().loadBlockStatementData(height);
			}

		private:
			BlockStorageCache m_cache;
		};

		// endregion

		// region MemoryTraits

		struct MemoryTraits {
			struct Guard {
				std::string name() const {
					return std::string();
				}
			};
			using StorageType = BlockStorageCacheToBlockStorageAdapter;

			static std::unique_ptr<StorageType> OpenStorage(const std::string&) {
				return std::make_unique<StorageType>(mocks::CreateMemoryBlockStorage(0));
			}

			static std::unique_ptr<StorageType> PrepareStorage(const std::string& destination, Height height = Height()) {
				if (Height() == height)
					return OpenStorage(destination);

				// set storage height to `height - 1` because next block saved will be at `height`
				// (real block needs to be saved at this height because BlockStorageCache will load it to populate MRU block cache)
				auto pBlock = test::GenerateBlockWithTransactions(5, height - Height(1));
				auto pRealStorage = mocks::CreateMemoryBlockStorage(0);
				pRealStorage->dropBlocksAfter(pBlock->Height - Height(1));
				pRealStorage->saveBlock(test::CreateBlockElementForSaveTests(*pBlock));
				return std::make_unique<StorageType>(std::move(pRealStorage));
			}
		};

		// endregion
	}

	DEFINE_BLOCK_STORAGE_TESTS(MemoryTraits)

	// note that these aren't really pure delegation tests because they call a member on the cache
	// and compare its behavior with that of the same call on the underlying storage

	namespace {
		constexpr uint32_t Delegation_Chain_Size = 15;
	}

	// region chainHeight

	TEST(TEST_CLASS, ChainHeightDelegatesToStorage) {
		// Arrange:
		BlockStorageCache cache(mocks::CreateMemoryBlockStorage(Delegation_Chain_Size), mocks::CreateMemoryBlockStorage(0));

		// Act:
		auto height = cache.view().chainHeight();

		// Assert:
		EXPECT_EQ(Height(Delegation_Chain_Size), height);
	}

	// endregion

	// region loadHashesFrom

	TEST(TEST_CLASS, LoadHashesFromDelegatesToStorage_SingleHash) {
		// Arrange:
		auto pStorage = mocks::CreateMemoryBlockStorage(Delegation_Chain_Size);
		auto pStorageRaw = pStorage.get();
		BlockStorageCache cache(std::move(pStorage), mocks::CreateMemoryBlockStorage(0));

		for (auto i = 1u; i <= Delegation_Chain_Size; ++i) {
			// Act:
			Height height(i);
			auto cacheHashes = cache.view().loadHashesFrom(height, 1);
			auto storageHashes = pStorageRaw->loadHashesFrom(height, 1);

			// Assert:
			ASSERT_EQ(1u, storageHashes.size());
			ASSERT_EQ(1u, cacheHashes.size());
			EXPECT_EQ(*storageHashes.cbegin(), *cacheHashes.cbegin());
		}
	}

	TEST(TEST_CLASS, LoadHashesFromDelegatesToStorage_AllHashes) {
		// Arrange:
		auto pStorage = mocks::CreateMemoryBlockStorage(Delegation_Chain_Size);
		auto pStorageRaw = pStorage.get();
		BlockStorageCache cache(std::move(pStorage), mocks::CreateMemoryBlockStorage(0));

		// Act:
		Height height(1);
		auto cacheHashes = cache.view().loadHashesFrom(height, Delegation_Chain_Size + 10);
		auto storageHashes = pStorageRaw->loadHashesFrom(height, Delegation_Chain_Size + 10);

		// Assert:
		ASSERT_EQ(Delegation_Chain_Size, storageHashes.size());
		ASSERT_EQ(Delegation_Chain_Size, cacheHashes.size());

		auto storageIter = storageHashes.cbegin();
		auto cacheIter = cacheHashes.cbegin();
		for (auto i = 1u; i <= Delegation_Chain_Size; ++i) {
			EXPECT_EQ(*storageIter, *cacheIter);
			++storageIter;
			++cacheIter;
		}
	}

	// endregion

	// region loadBlock(Element)

	TEST(TEST_CLASS, LoadBlockDelegatesToStorage) {
		// Arrange:
		auto pStorage = mocks::CreateMemoryBlockStorage(Delegation_Chain_Size);
		auto pStorageRaw = pStorage.get();
		BlockStorageCache cache(std::move(pStorage), mocks::CreateMemoryBlockStorage(0));

		for (auto i = 1u; i <= Delegation_Chain_Size; ++i) {
			// Act:
			Height height(i);
			auto pCacheBlock = cache.view().loadBlock(height);
			auto pStorageBlock = pStorageRaw->loadBlock(height);

			// Assert:
			EXPECT_EQ(*pStorageBlock, *pCacheBlock);
		}
	}

	TEST(TEST_CLASS, LoadBlockElementDelegatesToStorage) {
		// Arrange:
		auto pStorage = mocks::CreateMemoryBlockStorage(Delegation_Chain_Size);
		auto pStorageRaw = pStorage.get();
		BlockStorageCache cache(std::move(pStorage), mocks::CreateMemoryBlockStorage(0));

		for (auto i = 1u; i <= Delegation_Chain_Size; ++i) {
			// Act:
			Height height(i);

			auto pCacheBlockElement = cache.view().loadBlockElement(height);
			auto pStorageBlockElement = pStorageRaw->loadBlockElement(height);

			// Assert:
			test::AssertEqual(*pStorageBlockElement, *pCacheBlockElement);
		}
	}

	// endregion

	// region saveBlock - delegation

	namespace {
		template<typename TAction>
		void RunSaveDelegationTest(TAction action) {
			// Arrange:
			auto pStorage = mocks::CreateMemoryBlockStorage(Delegation_Chain_Size);
			auto pStorageRaw = pStorage.get();

			auto pStagingStorage = mocks::CreateMemoryBlockStorage(0);
			auto pStagingStorageRaw = pStagingStorage.get();

			BlockStorageCache cache(std::move(pStorage), std::move(pStagingStorage));

			// Act + Assert:
			action(cache, *pStorageRaw, *pStagingStorageRaw);
		}

		template<typename TSource>
		void AssertBlockInSource(
				const TSource& source,
				Height height,
				const model::Block& block,
				const Hash256& blockHash,
				const std::string& message) {
			// Assert:
			EXPECT_EQ(height, source.chainHeight()) << message;

			auto pSourceBlockElement = source.loadBlockElement(height);
			EXPECT_EQ(block, pSourceBlockElement->Block) << message;
			EXPECT_EQ(blockHash, pSourceBlockElement->EntityHash) << message;
		}
	}

	TEST(TEST_CLASS, SaveBlockDoesNotDelegateToPrimaryStorageWhenCommitIsNotCalled) {
		// Arrange:
		RunSaveDelegationTest([](auto& cache, const auto& storage, const auto& stagingStorage) {
			Height newBlockHeight(Delegation_Chain_Size + 1);
			auto pBlock = test::GenerateBlockWithTransactions(0, newBlockHeight);
			auto blockHash = test::GenerateRandomByteArray<Hash256>();

			// Act:
			cache.modifier().saveBlock(test::BlockToBlockElement(*pBlock, blockHash));

			// Assert: cache is not updated
			EXPECT_EQ(Height(Delegation_Chain_Size), cache.view().chainHeight());

			// - permanent storage is not updated
			EXPECT_EQ(Height(Delegation_Chain_Size), storage.chainHeight());

			// - staging storage is updated
			AssertBlockInSource(stagingStorage, newBlockHeight, *pBlock, blockHash, "stagingStorage");
		});
	}

	TEST(TEST_CLASS, SaveBlockDelegatesToPrimaryStorageWhenCommitIsCalled) {
		// Arrange:
		RunSaveDelegationTest([](auto& cache, const auto& storage, const auto& stagingStorage) {
			Height newBlockHeight(Delegation_Chain_Size + 1);
			auto pBlock = test::GenerateBlockWithTransactions(0, newBlockHeight);
			auto blockHash = test::GenerateRandomByteArray<Hash256>();

			// Act:
			{
				auto modifier = cache.modifier();
				modifier.saveBlock(test::BlockToBlockElement(*pBlock, blockHash));
				modifier.commit();
			}

			// Assert: cache is updated
			AssertBlockInSource(cache.view(), newBlockHeight, *pBlock, blockHash, "cache.view()");

			// - permanent storage is updated
			AssertBlockInSource(storage, newBlockHeight, *pBlock, blockHash, "storage");

			// - staging storage is pruned
			EXPECT_EQ(Height(0), stagingStorage.chainHeight());
		});
	}

	// endregion

	// region saveBlocks - delegation

	namespace {
		auto GenerateBlockElements(size_t numBlockElements) {
			std::vector<std::unique_ptr<model::Block>> blocks;
			std::vector<model::BlockElement> blockElements;
			for (auto i = 0u; i < numBlockElements; ++i) {
				blocks.push_back(test::GenerateBlockWithTransactions(0, Height(Delegation_Chain_Size + 1 + i)));
				blockElements.push_back(test::BlockToBlockElement(*blocks.back(), test::GenerateRandomByteArray<Hash256>()));
			}

			return std::make_pair(std::move(blocks), std::move(blockElements));
		}

		template<typename TSource>
		void AssertBlocksInSource(
				const TSource& source,
				Height height,
				const std::vector<model::BlockElement>& blockElements,
				const std::string& message) {
			// Assert:
			EXPECT_EQ(height, source.chainHeight()) << message;

			for (auto i = 0u; i < blockElements.size(); ++i) {
				auto pSourceBlockElement = source.loadBlockElement(height - Height(blockElements.size() - 1 - i));
				EXPECT_EQ(blockElements[i].Block, pSourceBlockElement->Block) << message << " at " << i;
				EXPECT_EQ(blockElements[i].EntityHash, pSourceBlockElement->EntityHash) << message << " at " << i;
			}
		}
	}

	TEST(TEST_CLASS, SaveBlocksDoesNotDelegateToPrimaryStorageWhenCommitIsNotCalled) {
		// Arrange:
		RunSaveDelegationTest([](auto& cache, const auto& storage, const auto& stagingStorage) {
			constexpr size_t Num_Block_Elements = 5;
			auto blockElements = GenerateBlockElements(Num_Block_Elements);
			auto newBlockHeight = Height(Delegation_Chain_Size + Num_Block_Elements);

			// Act:
			cache.modifier().saveBlocks(blockElements.second);

			// Assert: cache is not updated
			EXPECT_EQ(Height(Delegation_Chain_Size), cache.view().chainHeight());

			// - permanent storage is not updated
			EXPECT_EQ(Height(Delegation_Chain_Size), storage.chainHeight());

			// - staging storage is updated
			AssertBlocksInSource(stagingStorage, newBlockHeight, blockElements.second, "stagingStorage");
		});
	}

	TEST(TEST_CLASS, SaveBlocksDelegatesToPrimaryStorageWhenCommitIsCalled) {
		// Arrange:
		RunSaveDelegationTest([](auto& cache, const auto& storage, const auto& stagingStorage) {
			constexpr size_t Num_Block_Elements = 5;
			auto blockElements = GenerateBlockElements(Num_Block_Elements);
			auto newBlockHeight = Height(Delegation_Chain_Size + Num_Block_Elements);

			// Act:
			{
				auto modifier = cache.modifier();
				modifier.saveBlocks(blockElements.second);
				modifier.commit();
			}

			// Assert: cache is updated
			AssertBlocksInSource(cache.view(), newBlockHeight, blockElements.second, "cache.view()");

			// - permanent storage is updated
			AssertBlocksInSource(storage, newBlockHeight, blockElements.second, "storage");

			// - staging storage is pruned
			EXPECT_EQ(Height(0), stagingStorage.chainHeight());
		});
	}

	// endregion

	// region saveBlocks - other

	TEST(TEST_CLASS, SaveBlocksOutOfOrderThrows) {
		// Arrange:
		constexpr size_t Num_Block_Elements = 3;
		auto pStorage = mocks::CreateMemoryBlockStorage(Delegation_Chain_Size);
		BlockStorageCache cache(std::move(pStorage), mocks::CreateMemoryBlockStorage(0));
		std::vector<std::unique_ptr<model::Block>> blocks;
		std::vector<model::BlockElement> blockElements;
		for (auto i = 0u; i < Num_Block_Elements; ++i) {
			blocks.push_back(test::GenerateBlockWithTransactions(0, Height(Delegation_Chain_Size + i + 1)));
			blockElements.push_back(test::BlockToBlockElement(*blocks.back(), test::GenerateRandomByteArray<Hash256>()));
		}

		// - swap heights of two blocks
		blocks[1]->Height = Height(Delegation_Chain_Size + 3);
		blocks[2]->Height = Height(Delegation_Chain_Size + 2);

		// Act + Assert:
		EXPECT_THROW(cache.modifier().saveBlocks(blockElements), catapult_invalid_argument);
	}

	TEST(TEST_CLASS, SaveBlocksWithNoElementsIsNoOp) {
		// Arrange:
		auto pStorage = mocks::CreateMemoryBlockStorage(Delegation_Chain_Size);
		auto pStorageRaw = pStorage.get();
		BlockStorageCache cache(std::move(pStorage), mocks::CreateMemoryBlockStorage(0));

		// Act:
		cache.modifier().saveBlocks(std::vector<model::BlockElement>());

		// Assert:
		auto expectedChainHeight = Height(Delegation_Chain_Size);
		EXPECT_EQ(expectedChainHeight, cache.view().chainHeight());
		EXPECT_EQ(expectedChainHeight, pStorageRaw->chainHeight());
	}

	// endregion

	// region dropBlocksAfter - delegation

	TEST(TEST_CLASS, DropBlocksAfterDoesNotDelegateToPrimaryStorageWhenCommitIsNotCalled) {
		// Arrange:
		RunSaveDelegationTest([](auto& cache, const auto& storage, const auto& stagingStorage) {
			// Act:
			cache.modifier().dropBlocksAfter(Height(7));

			// Assert: cache is not updated
			EXPECT_EQ(Height(Delegation_Chain_Size), cache.view().chainHeight());

			// - permanent storage is not updated
			EXPECT_EQ(Height(Delegation_Chain_Size), storage.chainHeight());

			// - staging storage is updated
			EXPECT_EQ(Height(7), stagingStorage.chainHeight());
		});
	}

	TEST(TEST_CLASS, DropBlocksAfterDelegatesToPrimaryStorageWhenCommitIsCalled) {
		// Arrange:
		RunSaveDelegationTest([](auto& cache, const auto& storage, const auto& stagingStorage) {
			// Act:
			{
				auto modifier = cache.modifier();
				modifier.dropBlocksAfter(Height(7));
				modifier.commit();
			}

			// Assert: cache is updated
			EXPECT_EQ(Height(7), cache.view().chainHeight());

			// - permanent storage is updated
			EXPECT_EQ(Height(7), storage.chainHeight());

			// - staging storage is pruned
			EXPECT_EQ(Height(0), stagingStorage.chainHeight());
		});
	}

	// endregion

	// region commit

	TEST(TEST_CLASS, CanDropAndSaveBlocksWithSingleCommmit) {
		// Arrange:
		BlockStorageCache cache(mocks::CreateMemoryBlockStorage(12), mocks::CreateMemoryBlockStorage(0));

		// Sanity:
		EXPECT_EQ(Height(12), cache.view().chainHeight());

		// Act:
		auto pNewBlock = test::GenerateBlockWithTransactions(5, Height(9));
		auto newBlockElement = test::CreateBlockElementForSaveTests(*pNewBlock);
		{
			auto modifier = cache.modifier();
			modifier.dropBlocksAfter(Height(8));
			modifier.saveBlock(newBlockElement);
			modifier.commit();
		}

		// Assert:
		EXPECT_EQ(Height(9), cache.view().chainHeight());
		test::AssertEqual(newBlockElement, *cache.view().loadBlockElement(Height(9)));
	}

	// endregion

	// region synchronization

	namespace {
		auto CreateLockProvider() {
			return std::make_unique<BlockStorageCache>(mocks::CreateMemoryBlockStorage(7), mocks::CreateMemoryBlockStorage(0));
		}
	}

	DEFINE_LOCK_PROVIDER_TESTS(TEST_CLASS)

	// endregion
}}
