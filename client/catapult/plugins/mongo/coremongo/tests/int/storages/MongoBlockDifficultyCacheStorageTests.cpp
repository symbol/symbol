#include "src/storages/MongoBlockDifficultyCacheStorage.h"
#include "plugins/mongo/coremongo/src/MongoDatabase.h"
#include "plugins/mongo/coremongo/src/MongoDbStorage.h"
#include "catapult/cache/BlockDifficultyCache.h"
#include "catapult/model/BlockChainConfiguration.h"
#include "tests/test/cache/CacheTestUtils.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/test/mongo/MongoTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace mongo { namespace storages {

	namespace {
		constexpr uint64_t Multiple_Blocks_Count = 10;
		constexpr uint64_t Difficulty_History_Size = 100;

		class TestContext final : public test::PrepareDatabaseMixin {
		public:
			explicit TestContext(size_t topHeight)
					: PrepareDatabaseMixin(test::DatabaseName())
					, m_pConfig(test::CreateDefaultMongoStorageConfiguration(test::DatabaseName()))
					, m_pCacheStorage(CreateMongoBlockDifficultyCacheStorage(
							m_pConfig->createDatabaseConnection(),
							Difficulty_History_Size))
					, m_pDbStorage(std::make_unique<plugins::MongoDbStorage>(m_pConfig, test::CreateDefaultMongoTransactionRegistry())) {
				for (auto i = 1u; i <= topHeight; ++i) {
					auto transactions = test::GenerateRandomTransactions(10);
					m_blocks.push_back(test::GenerateRandomBlockWithTransactions(test::MakeConst(transactions)));
					m_blocks.back()->Height = Height(i);
					m_blockElements.emplace_back(test::BlockToBlockElement(*m_blocks.back(), test::GenerateRandomData<Hash256_Size>()));
				}

				saveBlocks();
			}

		public:
			plugins::ExternalCacheStorage& cacheStorage() {
				return *m_pCacheStorage;
			}

			void saveBlocks() {
				for (const auto& blockElement : m_blockElements)
					m_pDbStorage->saveBlock(blockElement);
			}

			const std::vector<model::BlockElement>& elements() {
				return m_blockElements;
			}

			int64_t numBlocksInCollection() {
				auto database = m_pConfig->createDatabaseConnection();
				auto blocks = database["blocks"];
				return blocks.count({});
			}

		private:
			std::vector<std::unique_ptr<model::Block>> m_blocks;
			std::vector<model::BlockElement> m_blockElements;
			std::shared_ptr<plugins::MongoStorageConfiguration> m_pConfig;
			std::unique_ptr<plugins::ExternalCacheStorage> m_pCacheStorage;
			std::unique_ptr<plugins::MongoDbStorage> m_pDbStorage;
		};

		auto ToBlockDifficultyInfo(const model::Block& block) {
			return state::BlockDifficultyInfo(block.Height, block.Timestamp, block.Difficulty);
		}

		template<typename TIter>
		void AssertDifficultyInfos(TIter startIter, TIter endIter, const cache::BlockDifficultyCacheView& infos) {
			auto numIterations = 0u;
			for (; endIter != startIter; ++startIter) {
				const auto& blockElement = *startIter;
				EXPECT_TRUE(infos.contains(ToBlockDifficultyInfo(blockElement.Block))) << "at block " << blockElement.Block.Height;
				++numIterations;
			}

			EXPECT_EQ(infos.size(), numIterations);
		}
	}

	// region saveDelta

	TEST(MongoBlockDifficultyCacheStorageTests, SaveDeltaIsNoOp) {
		// Arrange:
		TestContext context(Multiple_Blocks_Count);
		auto chainConfig = model::BlockChainConfiguration::Uninitialized();
		chainConfig.MaxDifficultyBlocks = Difficulty_History_Size;
		auto cache = test::CreateEmptyCatapultCache(chainConfig);
		context.cacheStorage().loadAll(cache, Height(Multiple_Blocks_Count));

		// Sanity:
		EXPECT_EQ(Multiple_Blocks_Count, context.numBlocksInCollection());

		auto delta = cache.createDelta();
		auto& blockDifficultyCacheDelta = delta.sub<cache::BlockDifficultyCache>();
		blockDifficultyCacheDelta.insert(Height(Multiple_Blocks_Count + 1), Timestamp(123), Difficulty());

		// Act:
		context.cacheStorage().saveDelta(delta);

		// Assert:
		EXPECT_EQ(Multiple_Blocks_Count, context.numBlocksInCollection());
	}

	// endregion

	// region loadAll

	TEST(MongoBlockDifficultyCacheStorageTests, CanLoadDifficulties) {
		// Arrange:
		TestContext context(Multiple_Blocks_Count);
		auto chainConfig = model::BlockChainConfiguration::Uninitialized();
		chainConfig.MaxDifficultyBlocks = Difficulty_History_Size;
		auto cache = test::CreateEmptyCatapultCache(chainConfig);

		// Act: load 6 difficulties
		context.cacheStorage().loadAll(cache, Height(6));

		// Assert:
		auto view = cache.createView();
		const auto& difficultyView = view.sub<cache::BlockDifficultyCache>();
		ASSERT_EQ(6u, difficultyView.size());
		auto start = context.elements().cbegin();
		auto end = context.elements().cbegin();
		std::advance(end, 6u);
		AssertDifficultyInfos(start, end, difficultyView);
	}

	TEST(MongoBlockDifficultyCacheStorageTests, LoadAllLoadsAtMostAvailableDifficulties) {
		// Arrange:
		TestContext context(Multiple_Blocks_Count);
		auto chainConfig = model::BlockChainConfiguration::Uninitialized();
		chainConfig.MaxDifficultyBlocks = Difficulty_History_Size;
		auto cache = test::CreateEmptyCatapultCache(chainConfig);

		// Act: load all difficulties
		context.cacheStorage().loadAll(cache, Height(15));

		// Assert:
		auto view = cache.createView();
		const auto& difficultyView = view.sub<cache::BlockDifficultyCache>();
		ASSERT_EQ(Multiple_Blocks_Count, difficultyView.size());
		auto start = context.elements().cbegin();
		auto end = context.elements().cend();
		AssertDifficultyInfos(start, end, difficultyView);
	}

	TEST(MongoBlockDifficultyCacheStorageTests, LoadAllDoesNotLoadAnyDifficultiesIfThereAreNoBlocks) {
		// Arrange:
		TestContext context(0);
		auto chainConfig = model::BlockChainConfiguration::Uninitialized();
		chainConfig.MaxDifficultyBlocks = Difficulty_History_Size;
		auto cache = test::CreateEmptyCatapultCache(chainConfig);

		// Act:
		context.cacheStorage().loadAll(cache, Height(15));

		// Assert:
		auto view = cache.createView();
		const auto& difficultyView = view.sub<cache::BlockDifficultyCache>();
		EXPECT_EQ(0u, difficultyView.size());
	}

	// endregion
}}}
