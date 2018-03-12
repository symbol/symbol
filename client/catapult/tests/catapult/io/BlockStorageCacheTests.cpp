#include "catapult/io/BlockStorageCache.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/test/core/mocks/MockMemoryBasedStorage.h"
#include "tests/test/nodeps/LockTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace io {

#define TEST_CLASS BlockStorageCacheTests

	namespace {
		// wraps a BlockStorageCache in a BlockStorage so that it can be tested via the tests in ChainStorageTests.h
		class BlockStorageCacheToBlockStorageAdapter : public BlockStorage {
		public:
			BlockStorageCacheToBlockStorageAdapter(std::unique_ptr<BlockStorage>&& pStorage)
					: m_cache(std::move(pStorage))
			{}

		public:
			Height chainHeight() const override {
				return m_cache.view().chainHeight();
			}

			std::shared_ptr<const model::Block> loadBlock(Height height) const override {
				return m_cache.view().loadBlock(height);
			}

			std::shared_ptr<const model::BlockElement> loadBlockElement(Height height) const override {
				return m_cache.view().loadBlockElement(height);
			}

			model::HashRange loadHashesFrom(Height height, size_t maxHashes) const override {
				return m_cache.view().loadHashesFrom(height, maxHashes);
			}

			void saveBlock(const model::BlockElement& blockElement) override {
				return m_cache.modifier().saveBlock(blockElement);
			}

			void dropBlocksAfter(Height height) override {
				return m_cache.modifier().dropBlocksAfter(height);
			}

		private:
			BlockStorageCache m_cache;
		};

		struct MemoryBasedGuard {
			std::string name() const {
				return std::string();
			}
		};

		struct MemoryBasedTraits {
			using Guard = MemoryBasedGuard;
			using StorageType = BlockStorageCacheToBlockStorageAdapter;

			static std::unique_ptr<StorageType> OpenStorage(const std::string&) {
				return std::make_unique<StorageType>(std::make_unique<mocks::MockMemoryBasedStorage>());
			}

			static std::unique_ptr<StorageType> PrepareStorage(const std::string& destination, Height height = Height()) {
				auto pStorage = OpenStorage(destination);

				if (Height() != height)
					// abuse drop blocks to fake current height...
					// note: since we will want to save next block at height, we need to drop all
					// after `height-1` due to check in saveBlock()
					pStorage->dropBlocksAfter(Height(height.unwrap() - 1));

				return pStorage;
			}
		};
	}
}}

#define STORAGE_TESTS_CLASS_NAME TEST_CLASS
#define STORAGE_TESTS_TRAITS_NAME MemoryBasedTraits

#include "BlockStorageTests.h"

#undef STORAGE_TESTS_TRAITS_NAME
#undef STORAGE_TESTS_CLASS_NAME

namespace catapult { namespace io {

#define TEST_CLASS BlockStorageCacheTests

	// note that these aren't really pure delegation tests because they call a member on the cache
	// and compare its behavior with that of the same call on the underlying storage

	namespace {
		constexpr uint32_t Delegation_Chain_Size = 15;
	}

	// region chainHeight

	TEST(TEST_CLASS, ChainHeightDelegatesToStorage) {
		// Arrange:
		BlockStorageCache cache(mocks::CreateMemoryBasedStorage(Delegation_Chain_Size));

		// Act:
		auto height = cache.view().chainHeight();

		// Assert:
		EXPECT_EQ(Height(Delegation_Chain_Size), height);
	}

	// endregion

	// region loadBlock

	TEST(TEST_CLASS, LoadBlockDelegatesToStorage) {
		// Arrange:
		auto pStorage = mocks::CreateMemoryBasedStorage(Delegation_Chain_Size);
		auto pStorageRaw = pStorage.get();
		BlockStorageCache cache(std::move(pStorage));

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
		auto pStorage = mocks::CreateMemoryBasedStorage(Delegation_Chain_Size);
		auto pStorageRaw = pStorage.get();
		BlockStorageCache cache(std::move(pStorage));

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

	// region loadHashes

	TEST(TEST_CLASS, LoadHashesFromDelegatesToStorage_SingleHash) {
		// Arrange:
		auto pStorage = mocks::CreateMemoryBasedStorage(Delegation_Chain_Size);
		auto pStorageRaw = pStorage.get();
		BlockStorageCache cache(std::move(pStorage));

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
		auto pStorage = mocks::CreateMemoryBasedStorage(Delegation_Chain_Size);
		auto pStorageRaw = pStorage.get();
		BlockStorageCache cache(std::move(pStorage));

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

	// region saveBlock(s)

	TEST(TEST_CLASS, SaveBlockDelegatesToStorage) {
		// Arrange:
		auto pStorage = mocks::CreateMemoryBasedStorage(Delegation_Chain_Size);
		auto pStorageRaw = pStorage.get();
		BlockStorageCache cache(std::move(pStorage));
		Height newBlockHeight(Delegation_Chain_Size + 1);
		auto pBlock = test::GenerateVerifiableBlockAtHeight(newBlockHeight);
		auto blockHash = test::GenerateRandomData<Hash256_Size>();

		// Act:
		cache.modifier().saveBlock(test::BlockToBlockElement(*pBlock, blockHash));

		// Assert: cache is updated
		EXPECT_EQ(newBlockHeight, cache.view().chainHeight());
		auto pCacheBlockElement = cache.view().loadBlockElement(newBlockHeight);
		EXPECT_EQ(*pBlock, pCacheBlockElement->Block);
		EXPECT_EQ(blockHash, pCacheBlockElement->EntityHash);

		// - underlying storage is updated
		EXPECT_EQ(newBlockHeight, pStorageRaw->chainHeight());
		auto pStorageBlockElement = pStorageRaw->loadBlockElement(newBlockHeight);
		EXPECT_EQ(*pBlock, pStorageBlockElement->Block);
		EXPECT_EQ(blockHash, pStorageBlockElement->EntityHash);
	}

	TEST(TEST_CLASS, SaveBlocksDelegatesToStorage) {
		// Arrange:
		constexpr size_t Num_Block_Elements = 5;
		auto pStorage = mocks::CreateMemoryBasedStorage(Delegation_Chain_Size);
		auto pStorageRaw = pStorage.get();
		BlockStorageCache cache(std::move(pStorage));
		std::vector<std::unique_ptr<model::Block>> blocks;
		std::vector<model::BlockElement> blockElements;
		for (auto i = 0u; i < Num_Block_Elements; ++i) {
			blocks.push_back(test::GenerateVerifiableBlockAtHeight(Height(Delegation_Chain_Size + i + 1)));
			blockElements.push_back(test::BlockToBlockElement(*blocks.back(), test::GenerateRandomData<Hash256_Size>()));
		}

		// Act:
		cache.modifier().saveBlocks(blockElements);

		// Assert: cache is updated
		auto expectedChainHeight = Height(Delegation_Chain_Size + Num_Block_Elements);
		EXPECT_EQ(expectedChainHeight, cache.view().chainHeight());
		for (auto i = 0u; i < Num_Block_Elements; ++i) {
			auto pCacheBlockElement = cache.view().loadBlockElement(Height(Delegation_Chain_Size + i + 1));
			EXPECT_EQ(blockElements[i].Block, pCacheBlockElement->Block);
			EXPECT_EQ(blockElements[i].EntityHash, pCacheBlockElement->EntityHash);
		}

		// - underlying storage is updated
		EXPECT_EQ(expectedChainHeight, pStorageRaw->chainHeight());
		for (auto i = 0u; i < Num_Block_Elements; ++i) {
			auto pStorageBlockElement = pStorageRaw->loadBlockElement(Height(Delegation_Chain_Size + i + 1));
			EXPECT_EQ(blockElements[i].Block, pStorageBlockElement->Block);
			EXPECT_EQ(blockElements[i].EntityHash, pStorageBlockElement->EntityHash);
		}
	}

	TEST(TEST_CLASS, SaveBlocksOutOfOrderThrows) {
		// Arrange:
		constexpr size_t Num_Block_Elements = 3;
		auto pStorage = mocks::CreateMemoryBasedStorage(Delegation_Chain_Size);
		BlockStorageCache cache(std::move(pStorage));
		std::vector<std::unique_ptr<model::Block>> blocks;
		std::vector<model::BlockElement> blockElements;
		for (auto i = 0u; i < Num_Block_Elements; ++i) {
			blocks.push_back(test::GenerateVerifiableBlockAtHeight(Height(Delegation_Chain_Size + i + 1)));
			blockElements.push_back(test::BlockToBlockElement(*blocks.back(), test::GenerateRandomData<Hash256_Size>()));
		}

		// - swap heights of two blocks
		blocks[1]->Height = Height(Delegation_Chain_Size + 3);
		blocks[2]->Height = Height(Delegation_Chain_Size + 2);

		// Act + Assert:
		EXPECT_THROW(cache.modifier().saveBlocks(blockElements), catapult_invalid_argument);
	}

	TEST(TEST_CLASS, SaveBlocksWithNoElementsIsNoOp) {
		// Arrange:
		auto pStorage = mocks::CreateMemoryBasedStorage(Delegation_Chain_Size);
		auto pStorageRaw = pStorage.get();
		BlockStorageCache cache(std::move(pStorage));

		// Act:
		cache.modifier().saveBlocks(std::vector<model::BlockElement>());

		// Assert:
		auto expectedChainHeight = Height(Delegation_Chain_Size);
		EXPECT_EQ(expectedChainHeight, cache.view().chainHeight());
		EXPECT_EQ(expectedChainHeight, pStorageRaw->chainHeight());
	}

	// endregion

	// region dropBlocksAfter

	TEST(TEST_CLASS, DropBlocksAfterDelegatesToStorage) {
		// Arrange:
		auto pStorage = mocks::CreateMemoryBasedStorage(Delegation_Chain_Size);
		auto pStorageRaw = pStorage.get();
		BlockStorageCache cache(std::move(pStorage));

		// Act:
		cache.modifier().dropBlocksAfter(Height(7));

		// Assert: cache is updated
		EXPECT_EQ(Height(7), cache.view().chainHeight());

		// - underlying storage is updated
		EXPECT_EQ(Height(7), pStorageRaw->chainHeight());
	}

	// endregion

	// region synchronization

	namespace {
		auto CreateLockProvider() {
			return std::make_unique<BlockStorageCache>(mocks::CreateMemoryBasedStorage(7));
		}
	}

	DEFINE_LOCK_PROVIDER_TESTS(TEST_CLASS)

	// endregion
}}
