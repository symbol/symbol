#include "catapult/io/AggregateBlockStorage.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/test/core/HashTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace io {

#define TEST_CLASS AggregateBlockStorageTests

	namespace {
		// region basic mocks

		class UnsupportedBlockStorage : public BlockStorage {
		public:
			Height chainHeight() const override {
				CATAPULT_THROW_RUNTIME_ERROR("chainHeight - not supported in mock");
			}

			std::shared_ptr<const model::Block> loadBlock(Height) const override {
				CATAPULT_THROW_RUNTIME_ERROR("loadBlock - not supported in mock");
			}

			std::shared_ptr<const model::BlockElement> loadBlockElement(Height) const override {
				CATAPULT_THROW_RUNTIME_ERROR("loadBlockElement - not supported in mock");
			}

			model::HashRange loadHashesFrom(Height, size_t) const override {
				CATAPULT_THROW_RUNTIME_ERROR("loadHashesFrom - not supported in mock");
			}

			void saveBlock(const model::BlockElement&) override {
				CATAPULT_THROW_RUNTIME_ERROR("saveBlock - not supported in mock");
			}

			void dropBlocksAfter(Height) override {
				CATAPULT_THROW_RUNTIME_ERROR("dropBlocksAfter - not supported in mock");
			}
		};

		class UnsupportedBlockChangeSubscriber : public BlockChangeSubscriber {
		public:
			void notifyBlock(const model::BlockElement&) override {
				CATAPULT_THROW_RUNTIME_ERROR("notifyBlock - not supported in mock");
			}

			void notifyDropBlocksAfter(Height) override {
				CATAPULT_THROW_RUNTIME_ERROR("notifyDropBlocksAfter - not supported in mock");
			}
		};

		// endregion

		template<typename TBlockStorage, typename TBlockChangeSubscriber = UnsupportedBlockChangeSubscriber>
		class TestContext {
		public:
			TestContext()
					: m_pStorage(std::make_unique<TBlockStorage>())
					, m_pStorageRaw(m_pStorage.get())
					, m_pSubscriber(std::make_unique<TBlockChangeSubscriber>())
					, m_pSubscriberRaw(m_pSubscriber.get())
					, m_pAggregate(CreateAggregateBlockStorage(std::move(m_pStorage), std::move(m_pSubscriber)))
			{}

		public:
			auto& storage() {
				return *m_pStorageRaw;
			}

			auto& subscriber() {
				return *m_pSubscriberRaw;
			}

			auto& aggregate() {
				return *m_pAggregate;
			}

		private:
			std::unique_ptr<TBlockStorage> m_pStorage; // notice that this is moved into m_pAggregate
			TBlockStorage* m_pStorageRaw;
			std::unique_ptr<TBlockChangeSubscriber> m_pSubscriber; // notice that this is moved into m_pAggregate
			TBlockChangeSubscriber* m_pSubscriberRaw;
			std::unique_ptr<BlockStorage> m_pAggregate;
		};
	}

	TEST(TEST_CLASS, ChainHeightDelegatesToStorage) {
		// Arrange:
		class MockBlockStorage : public UnsupportedBlockStorage {
		public:
			mutable size_t NumCalls = 0;

		public:
			Height chainHeight() const override {
				++NumCalls;
				return Height(123);
			}
		};

		TestContext<MockBlockStorage> context;

		// Act:
		auto height = context.aggregate().chainHeight();

		// Assert:
		EXPECT_EQ(1u, context.storage().NumCalls);
		EXPECT_EQ(Height(123), height);
	}

	// region loadBlock[Element]

	namespace {
		class MockBlockStorageBlockLoader : public UnsupportedBlockStorage {
		public:
			mutable std::vector<Height> Heights;
			std::shared_ptr<const model::Block> pBlock;
			std::shared_ptr<const model::BlockElement> pBlockElement;

		public:
			MockBlockStorageBlockLoader()
					: pBlock(test::GenerateEmptyRandomBlock())
					, pBlockElement(std::make_shared<model::BlockElement>(*pBlock))
			{}

		public:
			std::shared_ptr<const model::Block> loadBlock(Height height) const override {
				Heights.push_back(height);
				return pBlock;
			}

			std::shared_ptr<const model::BlockElement> loadBlockElement(Height height) const override {
				Heights.push_back(height);
				return pBlockElement;
			}
		};
	}

	TEST(TEST_CLASS, LoadBlockDelegatesToStorage) {
		// Arrange:
		TestContext<MockBlockStorageBlockLoader> context;

		// Act:
		auto pBlock = context.aggregate().loadBlock(Height(321));

		// Assert:
		ASSERT_EQ(1u, context.storage().Heights.size());
		EXPECT_EQ(Height(321), context.storage().Heights[0]);
		EXPECT_EQ(context.storage().pBlock, pBlock);
	}

	TEST(TEST_CLASS, LoadBlockElementDelegatesToStorage) {
		// Arrange:
		TestContext<MockBlockStorageBlockLoader> context;

		// Act:
		auto pBlockElement = context.aggregate().loadBlockElement(Height(321));

		// Assert:
		ASSERT_EQ(1u, context.storage().Heights.size());
		EXPECT_EQ(Height(321), context.storage().Heights[0]);
		EXPECT_EQ(context.storage().pBlockElement, pBlockElement);
	}

	// endregion

	TEST(TEST_CLASS, LoadHashesFromDelegatesToStorage) {
		// Arrange:
		class MockBlockStorage : public UnsupportedBlockStorage {
		public:
			mutable std::vector<std::pair<Height, size_t>> Params;
			model::HashRange Hashes;

		public:
			MockBlockStorage() : Hashes(test::GenerateRandomHashes(7))
			{}

		public:
			model::HashRange loadHashesFrom(Height height, size_t maxHashes) const override {
				Params.emplace_back(height, maxHashes);
				return model::HashRange::CopyRange(Hashes);
			}
		};

		TestContext<MockBlockStorage> context;

		// Act:
		auto hashes = context.aggregate().loadHashesFrom(Height(321), 776);

		// Assert:
		ASSERT_EQ(1u, context.storage().Params.size());
		EXPECT_EQ(Height(321), context.storage().Params[0].first);
		EXPECT_EQ(776u, context.storage().Params[0].second);

		// - the returned hashes should be equal to the generated hashes
		EXPECT_EQ(context.storage().Hashes.size(), hashes.size());
		EXPECT_EQ(context.storage().Hashes.size(), FindFirstDifferenceIndex(context.storage().Hashes, hashes));
	}

	TEST(TEST_CLASS, SaveBlockDelegatesToStorageAndPublisher) {
		// Arrange:
		class MockBlockStorage : public UnsupportedBlockStorage {
		public:
			std::vector<const model::BlockElement*> Elements;

		public:
			void saveBlock(const model::BlockElement& blockElement) override {
				Elements.push_back(&blockElement);
			}
		};

		class MockBlockChangeSubscriber : public UnsupportedBlockChangeSubscriber {
		public:
			std::vector<const model::BlockElement*> Elements;

		public:
			void notifyBlock(const model::BlockElement& blockElement) override {
				Elements.push_back(&blockElement);
			}
		};

		TestContext<MockBlockStorage, MockBlockChangeSubscriber> context;

		auto pBlock = test::GenerateEmptyRandomBlock();
		auto pBlockElement = std::make_shared<model::BlockElement>(*pBlock);

		// Act:
		context.aggregate().saveBlock(*pBlockElement);

		// Assert:
		ASSERT_EQ(1u, context.storage().Elements.size());
		EXPECT_EQ(pBlockElement.get(), context.storage().Elements[0]);

		ASSERT_EQ(1u, context.subscriber().Elements.size());
		EXPECT_EQ(pBlockElement.get(), context.subscriber().Elements[0]);
	}

	TEST(TEST_CLASS, DropBlocksAfterDelegatesToStorageAndPublisher) {
		// Arrange:
		class MockBlockStorage : public UnsupportedBlockStorage {
		public:
			std::vector<Height> Heights;

		public:
			void dropBlocksAfter(Height height) override {
				Heights.push_back(height);
			}
		};

		class MockBlockChangeSubscriber : public UnsupportedBlockChangeSubscriber {
		public:
			std::vector<Height> Heights;

		public:
			void notifyDropBlocksAfter(Height height) override {
				Heights.push_back(height);
			}
		};

		TestContext<MockBlockStorage, MockBlockChangeSubscriber> context;

		// Act:
		context.aggregate().dropBlocksAfter(Height(553));

		// Assert:
		ASSERT_EQ(1u, context.storage().Heights.size());
		EXPECT_EQ(Height(553), context.storage().Heights[0]);

		ASSERT_EQ(1u, context.subscriber().Heights.size());
		EXPECT_EQ(Height(553), context.subscriber().Heights[0]);
	}
}}
