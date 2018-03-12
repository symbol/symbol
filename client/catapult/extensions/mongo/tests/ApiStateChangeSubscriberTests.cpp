#include "mongo/src/ApiStateChangeSubscriber.h"
#include "catapult/model/ChainScore.h"
#include "tests/TestHarness.h"

namespace catapult { namespace mongo {

#define TEST_CLASS ApiStateChangeSubscriberTests

	namespace {
		// region basic mocks

		class MockChainScoreProvider : public ChainScoreProvider {
		public:
			const auto& scores() const {
				return m_scores;
			}

		public:
			void saveScore(const model::ChainScore& chainScore) override {
				m_scores.push_back(chainScore);
			}

			model::ChainScore loadScore() const override {
				CATAPULT_THROW_RUNTIME_ERROR("loadScore - not supported in mock");
			}

		private:
			std::vector<model::ChainScore> m_scores;
		};

		class MockExternalCacheStorage : public ExternalCacheStorage {
		public:
			MockExternalCacheStorage() : ExternalCacheStorage("MockExternalCacheStorage", std::numeric_limits<size_t>::max())
			{}

		public:
			const auto& deltas() const {
				return m_deltas;
			}

		public:
			void saveDelta(const cache::CatapultCacheDelta& cache) override {
				m_deltas.push_back(&cache);
			}

			void loadAll(cache::CatapultCache&, Height) const override {
				CATAPULT_THROW_RUNTIME_ERROR("loadAll - not supported in mock");
			}

		private:
			std::vector<const cache::CatapultCacheDelta*> m_deltas;
		};

		// endregion

		class TestContext {
		public:
			TestContext()
					: m_pChainScoreProvider(std::make_unique<MockChainScoreProvider>())
					, m_pChainScoreProviderRaw(m_pChainScoreProvider.get())
					, m_pExternalCacheStorage(std::make_unique<MockExternalCacheStorage>())
					, m_pExternalCacheStorageRaw(m_pExternalCacheStorage.get())
					, m_subscriber(std::move(m_pChainScoreProvider), std::move(m_pExternalCacheStorage))
			{}

		public:
			auto& chainScoreProvider() {
				return *m_pChainScoreProviderRaw;
			}

			auto& externalCacheStorage() {
				return *m_pExternalCacheStorageRaw;
			}

			auto& subscriber() {
				return m_subscriber;
			}

		private:
			std::unique_ptr<MockChainScoreProvider> m_pChainScoreProvider; // notice that this is moved into m_subscriber
			MockChainScoreProvider* m_pChainScoreProviderRaw;
			std::unique_ptr<MockExternalCacheStorage> m_pExternalCacheStorage; // notice that this is moved into m_subscriber
			MockExternalCacheStorage* m_pExternalCacheStorageRaw;
			ApiStateChangeSubscriber m_subscriber;
		};
	}

	TEST(TEST_CLASS, NotifyScoreChangeForwardsToChainScoreProvider) {
		// Arrange:
		TestContext context;
		auto chainScore = model::ChainScore(123, 435);

		// Act:
		context.subscriber().notifyScoreChange(chainScore);

		// Assert:
		ASSERT_EQ(1u, context.chainScoreProvider().scores().size());
		EXPECT_EQ(chainScore, context.chainScoreProvider().scores()[0]);

		EXPECT_TRUE(context.externalCacheStorage().deltas().empty());
	}

	TEST(TEST_CLASS, NotifyStateChangeForwardsToExternalCacheStorage) {
		// Arrange:
		TestContext context;
		auto cache = cache::CatapultCache({});
		auto cacheDelta = cache.createDelta();
		auto chainScore = model::ChainScore(123, 435);

		// Act:
		context.subscriber().notifyStateChange(consumers::StateChangeInfo(cacheDelta, chainScore, Height(123)));

		// Assert:
		EXPECT_TRUE(context.chainScoreProvider().scores().empty());

		ASSERT_EQ(1u, context.externalCacheStorage().deltas().size());
		EXPECT_EQ(&cacheDelta, context.externalCacheStorage().deltas()[0]);
	}
}}
