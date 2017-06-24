#include "src/MongoAggregateStorage.h"
#include "tests/TestHarness.h"

namespace catapult { namespace mongo { namespace plugins {

	namespace {
		struct StatsBlockStorage {
			size_t NumChainHeightCalls;
			Height LoadBlockHeight;
			Height LoadBlockElementHeight;
			Height LoadHashesFromHeight;
			size_t LoadHashesFromSize;
			const model::BlockElement* pSaveBlockBlockElement;
			Height DropBlocksAfterHeight;
			Height PruneBlocksBeforeHeight;
		};

		class CountingBlockStorage final : public io::PrunableBlockStorage {
		public:
			explicit CountingBlockStorage(StatsBlockStorage& stats) : m_stats(stats)
			{}

		public:
			Height chainHeight() const override {
				++m_stats.NumChainHeightCalls;
				return Height();
			}

			std::shared_ptr<const model::Block> loadBlock(Height height) const override {
				m_stats.LoadBlockHeight = height;
				return nullptr;
			}

			std::shared_ptr<const model::BlockElement> loadBlockElement(Height height) const override {
				m_stats.LoadBlockElementHeight = height;
				return nullptr;
			}

			model::HashRange loadHashesFrom(Height height, size_t maxHashes) const override {
				m_stats.LoadHashesFromHeight = height;
				m_stats.LoadHashesFromSize = maxHashes;
				return model::HashRange();
			}

			void saveBlock(const model::BlockElement& block) override {
				m_stats.pSaveBlockBlockElement = &block;
			}

			void dropBlocksAfter(Height height) override {
				m_stats.DropBlocksAfterHeight = height;
			}

			void pruneBlocksBefore(Height height) override {
				m_stats.PruneBlocksBeforeHeight = height;
			}

		private:
			StatsBlockStorage& m_stats;
		};

		class TestContext {
		public:
			explicit TestContext(uint32_t maxRollbackSize = 10)
					: m_fileStorageStats()
					, m_mongoStorageStats()
					, m_pStorage(std::make_shared<MongoAggregateStorage>(
							std::make_shared<CountingBlockStorage>(m_mongoStorageStats),
							std::make_shared<CountingBlockStorage>(m_fileStorageStats),
							maxRollbackSize))
			{}

			MongoAggregateStorage& aggregate() const {
				return *m_pStorage;
			}

			const StatsBlockStorage& file() const {
				return m_fileStorageStats;
			}

			const StatsBlockStorage& mongo() const {
				return m_mongoStorageStats;
			}

		private:
			StatsBlockStorage m_fileStorageStats;
			StatsBlockStorage m_mongoStorageStats;
			std::shared_ptr<MongoAggregateStorage> m_pStorage;
		};
	}

	// region BlockStorage interface

	TEST(MongoAggregateStorageTests, ChainHeightForwardsToMongoStorage) {
		// Arrange:
		TestContext context;

		// Act:
		context.aggregate().chainHeight();

		// Assert:
		EXPECT_EQ(0u, context.file().NumChainHeightCalls);
		EXPECT_EQ(1u, context.mongo().NumChainHeightCalls);
	}

	TEST(MongoAggregateStorageTests, LoadBlockForwardsToFileStorage) {
		// Arrange:
		TestContext context;

		// Act:
		context.aggregate().loadBlock(Height(12345));

		// Assert:
		EXPECT_EQ(Height(12345u), context.file().LoadBlockHeight);
		EXPECT_EQ(Height(0), context.mongo().LoadBlockHeight);
	}

	TEST(MongoAggregateStorageTests, LoadBlockElementForwardsToFileStorage) {
		// Arrange:
		TestContext context;

		// Act:
		context.aggregate().loadBlockElement(Height(12345));

		// Assert:
		EXPECT_EQ(Height(12345u), context.file().LoadBlockElementHeight);
		EXPECT_EQ(Height(0), context.mongo().LoadBlockElementHeight);
	}

	TEST(MongoAggregateStorageTests, LoadHashesFromForwardsToMongoStorage) {
		// Arrange:
		TestContext context;

		// Act:
		context.aggregate().loadHashesFrom(Height(12345), 321);

		// Assert:
		EXPECT_EQ(Height(0), context.file().LoadHashesFromHeight);
		EXPECT_EQ(0u, context.file().LoadHashesFromSize);
		EXPECT_EQ(Height(12345), context.mongo().LoadHashesFromHeight);
		EXPECT_EQ(321u, context.mongo().LoadHashesFromSize);
	}

	TEST(MongoAggregateStorageTests, SaveBlockForwardsToBoth) {
		// Arrange:
		TestContext context(360);
		model::Block block;
		model::BlockElement blockElement(block);
		block.Height = Height(123);

		// Act:
		context.aggregate().saveBlock(blockElement);

		// Assert:
		EXPECT_EQ(&blockElement, context.file().pSaveBlockBlockElement);
		EXPECT_EQ(&blockElement, context.mongo().pSaveBlockBlockElement);
		EXPECT_EQ(Height(0), context.file().PruneBlocksBeforeHeight);
	}

	TEST(MongoAggregateStorageTests, SaveNemesisBlockForwardsToMongoStorageOnly) {
		// Arrange:
		TestContext context(360);
		model::Block block;
		model::BlockElement blockElement(block);
		block.Height = Height(1);

		// Act:
		context.aggregate().saveBlock(blockElement);

		// Assert:
		EXPECT_FALSE(!!context.file().pSaveBlockBlockElement);
		EXPECT_EQ(&blockElement, context.mongo().pSaveBlockBlockElement);
		EXPECT_EQ(Height(0), context.file().PruneBlocksBeforeHeight);
	}

	namespace {
		void SaveBlock(const TestContext& context, Height height) {
			model::Block block;
			model::BlockElement blockElement(block);
			block.Height = height;
			context.aggregate().saveBlock(blockElement);
		}
	}

	TEST(MongoAggregateStorageTests, SaveBlockDoesNotForwardToPruneIfHeightIsEqualToRewrite) {
		// Arrange:
		TestContext context(100);

		// Act:
		SaveBlock(context, Height(100));

		// Assert:
		EXPECT_EQ(Height(0), context.file().PruneBlocksBeforeHeight);
	}

	TEST(MongoAggregateStorageTests, SaveBlockForwardsToPruneIfHeightIsLargerThanRewrite) {
		// Arrange:
		TestContext context(100);

		// Act:
		SaveBlock(context, Height(101));

		// Assert:
		EXPECT_EQ(Height(1), context.file().PruneBlocksBeforeHeight);
	}

	TEST(MongoAggregateStorageTests, SaveBlockForwardsToProperPrune) {
		// Arrange:
		TestContext context(1234);

		// Act:
		SaveBlock(context, Height(5000));

		// Assert:
		EXPECT_EQ(Height(5000 - 1234), context.file().PruneBlocksBeforeHeight);
	}

	TEST(MongoAggregateStorageTests, DropBlocksAfterForwardsToBoth) {
		// Arrange:
		TestContext context;

		// Act:
		context.aggregate().dropBlocksAfter(Height(12345));

		// Assert:
		EXPECT_EQ(Height(12345), context.file().DropBlocksAfterHeight);
		EXPECT_EQ(Height(12345), context.mongo().DropBlocksAfterHeight);
	}

	// endregion
}}}
