#pragma once
#include "catapult/model/BlockUtils.h"
#include "catapult/utils/HexFormatter.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/test/core/TransactionTestUtils.h"
#include "tests/test/core/mocks/MockMemoryBasedStorage.h"
#include "tests/test/nodeps/Nemesis.h"
#include <numeric>

namespace catapult { namespace io {

	namespace {
		std::unique_ptr<model::Block> CreateRandomBlock(Height height) {
			constexpr auto Network_Identifier = model::NetworkIdentifier::Zero;

			model::PreviousBlockContext context;
			test::FillWithRandomData(context.BlockHash);
			test::FillWithRandomData(context.GenerationHash);

			auto signer = test::GenerateKeyPair();
			auto transactions = test::GenerateRandomTransactions(5);
			auto pBlock = model::CreateBlock(context, Network_Identifier, signer.publicKey(), test::MakeConst(transactions));
			pBlock->Height = height;
			test::SignBlock(signer, *pBlock);
			return pBlock;
		}

		void SeedBlocks(BlockStorage& storage, Height startHeight, Height endHeight) {
			for (auto height = startHeight; height <= endHeight; height = height + Height(1)) {
				auto pBlock = CreateRandomBlock(height);
				auto blockElement = test::BlockToBlockElement(*pBlock);
				blockElement.EntityHash = Hash256();
				blockElement.EntityHash[Hash256_Size - 1] = static_cast<uint8_t>(height.unwrap());
				storage.saveBlock(blockElement);
			}
		}

		void SeedBlocks(BlockStorage& storage, size_t numBlocks) {
			SeedBlocks(storage, Height(2), Height(numBlocks));
		}

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

		template<typename TTraits>
		auto PrepareStorageWithBlocks(size_t numBlocks) {
			StorageContext<TTraits> context;
			context.pTempDirectoryGuard = std::make_unique<typename TTraits::Guard>();
			context.pStorage = TTraits::PrepareStorage(context.pTempDirectoryGuard->name());
			SeedBlocks(*context.pStorage, numBlocks);
			return context;
		}

		struct LoadBlockElementTraits {
			static auto Load(const BlockStorage& storage, Height height) {
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

			static void AssertLoadError(const BlockStorage& storage, Height height) {
				// Act + Assert:
				EXPECT_THROW(Load(storage, height), catapult_invalid_argument);
			}
		};

		struct LoadBlockTraits {
			static auto Load(const BlockStorage& storage, Height height) {
				return storage.loadBlock(height);
			}

			static void Assert(const model::Block& block, const Hash256&, const std::shared_ptr<const model::Block>& pBlock) {
				EXPECT_EQ(block.Signature, pBlock->Signature);
				EXPECT_EQ(block, *pBlock);
			}

			static void AssertLoadError(const BlockStorage& storage, Height height) {
				EXPECT_THROW(Load(storage, height), catapult_invalid_argument);
			}
		};

		struct LoadHashesTraits {
			static auto Load(const BlockStorage& storage, Height height) {
				return storage.loadHashesFrom(height, 1);
			}

			static void Assert(const model::Block&, const Hash256& expectedHash, const model::HashRange& hashes) {
				// Assert:
				EXPECT_EQ(1u, hashes.size());
				EXPECT_EQ(expectedHash, *hashes.cbegin());
			}

			static void AssertLoadError(const BlockStorage& storage, Height height) {
				// Assert:
				auto result = Load(storage, height);
				EXPECT_EQ(0u, result.size());
			}
		};
	}

#define LOAD_TRAITS_BASED_TEST(TEST_NAME) \
	template<typename TTraits, typename TLoadTraits> void TRAITS_TEST_NAME(STORAGE_TESTS_CLASS_NAME, TEST_NAME)(); \
	TEST(STORAGE_TESTS_CLASS_NAME, TEST_NAME##_Block) { \
		TRAITS_TEST_NAME(STORAGE_TESTS_CLASS_NAME, TEST_NAME)<STORAGE_TESTS_TRAITS_NAME, LoadBlockTraits>(); \
	} \
	TEST(STORAGE_TESTS_CLASS_NAME, TEST_NAME##_BlockElement) { \
		TRAITS_TEST_NAME(STORAGE_TESTS_CLASS_NAME, TEST_NAME)<STORAGE_TESTS_TRAITS_NAME, LoadBlockElementTraits>(); \
	} \
	TEST(STORAGE_TESTS_CLASS_NAME, TEST_NAME##_Hashes) { \
		TRAITS_TEST_NAME(STORAGE_TESTS_CLASS_NAME, TEST_NAME)<STORAGE_TESTS_TRAITS_NAME, LoadHashesTraits>(); \
	} \
	template<typename TTraits, typename TLoadTraits> void TRAITS_TEST_NAME(STORAGE_TESTS_CLASS_NAME, TEST_NAME)()

#define TRAITS_BASED_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(STORAGE_TESTS_CLASS_NAME, TEST_NAME)(); \
	TEST(STORAGE_TESTS_CLASS_NAME, TEST_NAME) { TRAITS_TEST_NAME(STORAGE_TESTS_CLASS_NAME, TEST_NAME)<STORAGE_TESTS_TRAITS_NAME>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(STORAGE_TESTS_CLASS_NAME, TEST_NAME)()

	TRAITS_BASED_TEST(StorageSeedInitiallyContainsNemesisBlock) {
		// Arrange:
#ifdef SIGNATURE_SCHEME_NIS1
		constexpr auto Source_Directory = "../seed/mijin-test.nis1";
#else
		constexpr auto Source_Directory = "../seed/mijin-test";
#endif

		const auto* pNemesisBlock = reinterpret_cast<const model::Block*>(&mocks::MemoryBasedStorage_NemesisBlockData);
		auto nemesisBlockElement = test::BlockToBlockElement(*pNemesisBlock);
		nemesisBlockElement.GenerationHash = test::GetNemesisGenerationHash();

		// Act:
		const auto pStorage = TTraits::OpenStorage(Source_Directory);
		auto pBlockElement = pStorage->loadBlockElement(Height(1));

		// Assert:
		EXPECT_EQ(Height(1), pStorage->chainHeight());
		test::AssertEqual(nemesisBlockElement, *pBlockElement);
		EXPECT_TRUE(model::VerifyBlockHeaderSignature(pBlockElement->Block));
	}

	TRAITS_BASED_TEST(SavingBlockWithHeightHigherThanChainHeightAltersChainHeight) {
		// Arrange:
		auto pStorage = PrepareStorageWithBlocks<TTraits>(10);

		// - save the initial block
		auto pNewBlock = CreateRandomBlock(Height(11));
		pStorage->saveBlock(test::BlockToBlockElement(*pNewBlock));

		// Sanity:
		EXPECT_EQ(Height(11), pStorage->chainHeight());

		// Act:
		pNewBlock->Height = Height(12);
		pStorage->saveBlock(test::BlockToBlockElement(*pNewBlock));

		// Assert:
		EXPECT_EQ(Height(12), pStorage->chainHeight());
	}

	namespace {
		model::BlockElement CreateBlockElementForSaveTests(const model::Block& block) {
			// Arrange: create a block element with a random hash
			auto blockElement = test::BlockToBlockElement(block, test::GenerateRandomData<Hash256_Size>());

			// - give the first transaction a random hash too (the random hash should be saved)
			blockElement.Transactions[0].EntityHash = test::GenerateRandomData<Hash256_Size>();
			return blockElement;
		}
	}

	TRAITS_BASED_TEST(CanOverwriteBlockWithSameData) {
		// Arrange:
		auto pStorage = PrepareStorageWithBlocks<TTraits>(10);

		auto pBlock = CreateRandomBlock(Height(11));
		auto expectedBlockElement = CreateBlockElementForSaveTests(*pBlock);
		pStorage->saveBlock(expectedBlockElement);

		// Act: drop and save the same block again
		pStorage->dropBlocksAfter(Height(10));
		pStorage->saveBlock(expectedBlockElement);
		auto pBlockElement = pStorage->loadBlockElement(Height(11));

		// Assert:
		EXPECT_EQ(Height(11), pStorage->chainHeight());
		test::AssertEqual(expectedBlockElement, *pBlockElement);
	}

	TRAITS_BASED_TEST(CanOverwriteBlockWithDifferentData) {
		// Arrange:
		auto pStorage = PrepareStorageWithBlocks<TTraits>(10);

		auto pBlock = CreateRandomBlock(Height(11));
		pStorage->saveBlock(test::BlockToBlockElement(*pBlock));

		auto expectedBlockElement = CreateBlockElementForSaveTests(*pBlock);

		// Act: drop, modify the block, and save it
		pStorage->dropBlocksAfter(Height(10));
		pBlock->Timestamp = pBlock->Timestamp + Timestamp(1);

		pStorage->saveBlock(expectedBlockElement);
		auto pBlockElement = pStorage->loadBlockElement(Height(11));

		// Assert: the modified data was loaded
		EXPECT_EQ(Height(11), pStorage->chainHeight());
		test::AssertEqual(expectedBlockElement, *pBlockElement);
	}

	namespace {
		template<typename TTraits>
		void AssertCannotSaveBlockAtHeight(size_t numSeedBlocks, Height newBlockHeight) {
			// Arrange:
			auto pStorage = PrepareStorageWithBlocks<TTraits>(numSeedBlocks);

			// Act + Assert:
			auto pNewBlock = CreateRandomBlock(newBlockHeight);
			EXPECT_THROW(pStorage->saveBlock(test::BlockToBlockElement(*pNewBlock)), catapult_invalid_argument);
		}
	}

	TRAITS_BASED_TEST(CannotSaveBlockWithHeightLessThanChainHeight) {
		// Assert:
		AssertCannotSaveBlockAtHeight<TTraits>(10, Height(1));
		AssertCannotSaveBlockAtHeight<TTraits>(10, Height(9));
	}

	TRAITS_BASED_TEST(CannotSaveBlockAtChainHeight) {
		// Assert:
		AssertCannotSaveBlockAtHeight<TTraits>(10, Height(10));
	}

	TRAITS_BASED_TEST(CannotSaveBlockMoreThanOneHeightBeyondChainHeight) {
		// Assert:
		AssertCannotSaveBlockAtHeight<TTraits>(10, Height(12));
		AssertCannotSaveBlockAtHeight<TTraits>(10, Height(110));
	}

	LOAD_TRAITS_BASED_TEST(CanLoadAtHeightLessThanChainHeight) {
		// Arrange:
		auto pStorage = PrepareStorageWithBlocks<TTraits>(10);

		auto pBlock = CreateRandomBlock(Height(11));
		auto expectedHash = test::GenerateRandomData<Hash256_Size>();
		pStorage->saveBlock(test::BlockToBlockElement(*pBlock, expectedHash));
		pStorage->saveBlock(test::BlockToBlockElement(*CreateRandomBlock(Height(12))));

		// Act:
		auto result = TLoadTraits::Load(*pStorage, Height(11));

		// Assert:
		EXPECT_EQ(Height(12), pStorage->chainHeight());
		TLoadTraits::Assert(*pBlock, expectedHash, result);
	}

	LOAD_TRAITS_BASED_TEST(CanLoadAtChainHeight) {
		// Arrange:
		auto pStorage = PrepareStorageWithBlocks<TTraits>(10);

		auto pBlock = CreateRandomBlock(Height(11));
		auto expectedHash = test::GenerateRandomData<Hash256_Size>();
		pStorage->saveBlock(test::BlockToBlockElement(*pBlock, expectedHash));

		// Act:
		auto result = TLoadTraits::Load(*pStorage, Height(11));

		// Assert:
		EXPECT_EQ(Height(11), pStorage->chainHeight());
		TLoadTraits::Assert(*pBlock, expectedHash, result);
	}

	LOAD_TRAITS_BASED_TEST(CannotLoadAtHeightGreaterThanChainHeight) {
		// Arrange:
		auto pStorage = PrepareStorageWithBlocks<TTraits>(10);

		// Act + Assert:
		TLoadTraits::AssertLoadError(*pStorage, Height(11));
	}

	LOAD_TRAITS_BASED_TEST(CanLoadMultipleSaved) {
		// Arrange:
		auto pStorage = PrepareStorageWithBlocks<TTraits>(5);

		auto pBlock1 = CreateRandomBlock(Height(6));
		auto pBlock2 = CreateRandomBlock(Height(7));
		auto expectedHash1 = test::GenerateRandomData<Hash256_Size>();
		auto expectedHash2 = test::GenerateRandomData<Hash256_Size>();
		pStorage->saveBlock(test::BlockToBlockElement(*pBlock1, expectedHash1));
		pStorage->saveBlock(test::BlockToBlockElement(*pBlock2, expectedHash2));

		// Act:
		auto result1 = TLoadTraits::Load(*pStorage, Height(6));
		auto result2 = TLoadTraits::Load(*pStorage, Height(7));

		// Assert:
		EXPECT_EQ(Height(7), pStorage->chainHeight());
		TLoadTraits::Assert(*pBlock1, expectedHash1, result1);
		TLoadTraits::Assert(*pBlock2, expectedHash2, result2);
	}

	TRAITS_BASED_TEST(CanDropBlocksAfterHeight) {
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

	namespace {
		void AssertNoHashes(uint32_t numBlocks, uint32_t maxHashes, Height requestHeight) {
			// Arrange:
			auto pStorage = mocks::CreateMemoryBasedStorage(numBlocks);

			// Act:
			auto hashes = pStorage->loadHashesFrom(requestHeight, maxHashes);

			// Assert:
			EXPECT_TRUE(hashes.empty());
		}
	}

	TRAITS_BASED_TEST(LoadHashesFrom_LoadsZeroHashesWhenRequestHeightIsZero) {
		AssertNoHashes(10, 1, Height(0));
	}

	TRAITS_BASED_TEST(LoadHashesFrom_LoadsZeroHashesWhenRequestHeightIsLargerThanLocalHeight) {
		AssertNoHashes(10, 1, Height(11));
		AssertNoHashes(10, 5, Height(23));
		AssertNoHashes(10, 5, Height(100));
	}

	namespace {
		template<typename TTraits>
		void AssertCanLoadHashes(
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
	}

	TRAITS_BASED_TEST(LoadHashesFrom_CanLoadASingleHash) {
		AssertCanLoadHashes<TTraits>(10u, 1u, Height(5), { 5 });
	}

	TRAITS_BASED_TEST(LoadHashesFrom_CanLoadLastHash) {
		AssertCanLoadHashes<TTraits>(10u, 1u, Height(10), { 10 });
		AssertCanLoadHashes<TTraits>(10u, 5u, Height(10), { 10 });
	}

	TRAITS_BASED_TEST(LoadHashesFrom_LoadsAtMostMaxHashes) {
		AssertCanLoadHashes<TTraits>(10u, 6u, Height(2), { 2, 3, 4, 5, 6, 7 });
	}

	TRAITS_BASED_TEST(LoadHashesFrom_LoadsAreBoundedByLastBlock) {
		AssertCanLoadHashes<TTraits>(10u, 10u, Height(5), { 5, 6, 7, 8, 9, 10 });
	}

	// running this test does not make any sense, as it needs to produce quite some blocks
	TRAITS_BASED_TEST(LoadHashesFrom_LoadsCanCrossIndexFileBoundary) {
		// Arrange: (note hashes are set inside SeedBlocks)
		std::vector<uint8_t> expectedHashes(11);
		std::iota(expectedHashes.begin(), expectedHashes.end(), static_cast<uint8_t>(65530 % 0xFF));

		StorageContext<TTraits> context;
		context.pTempDirectoryGuard = std::make_unique<typename TTraits::Guard>();
		context.pStorage = TTraits::PrepareStorage(context.pTempDirectoryGuard->name(), Height(65530));
		SeedBlocks(*context.pStorage, Height(65530), Height(65540));

		// Act:
		auto hashes = context.pStorage->loadHashesFrom(Height(65530), 100u);

		// Assert:
		ASSERT_EQ(expectedHashes.size(), hashes.size());
		auto i = 0u;
		for (const auto& hash : hashes) {
			auto expectedHash = Hash256();
			expectedHash[Hash256_Size - 1] = expectedHashes[i++];
			EXPECT_EQ(expectedHash, hash);
		}
	}
}}
