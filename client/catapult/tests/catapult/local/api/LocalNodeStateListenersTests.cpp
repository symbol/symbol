#include "catapult/local/api/LocalNodeStateListeners.h"
#include "catapult/cache/CatapultCache.h"
#include "catapult/consumers/StateChangeInfo.h"
#include "catapult/model/BlockChainConfiguration.h"
#include "catapult/model/ChainScore.h"
#include "tests/test/cache/CacheTestUtils.h"
#include "tests/test/nodeps/ParamsCapture.h"
#include "tests/TestHarness.h"

namespace catapult { namespace local { namespace api {

	namespace {
		struct SubscribeParams {
			model::ChainScore Score;
			bool IsMarkedCache;
		};

		class TestContext : public test::ParamsCapture<SubscribeParams> {
		public:
			TestContext() : m_cache(test::CreateCatapultCacheWithMarkerAccount())
			{}

			void subscribeDefault() {
				m_listeners.subscribeStateChange([this](const auto& score, const auto& stateChangeInfo) {
					this->push(SubscribeParams{ score, test::IsMarkedCache(stateChangeInfo.CacheDelta) });
				});
			}

			void subscribe(const StateChangeListener& stateChangeListener) {
				m_listeners.subscribeStateChange(stateChangeListener);
			}

			void notifyScore(model::ChainScore chainScore) {
				auto delta = m_cache.createDelta();
				consumers::StateChangeInfo stateChangeInfo(delta, model::ChainScore(), Height());

				listeners().notifyStateChange(chainScore, stateChangeInfo);
			}

			const LocalNodeStateListeners& listeners() const {
				return m_listeners;
			}

		private:
			cache::CatapultCache m_cache;
			LocalNodeStateListeners m_listeners;
		};
	}

	TEST(LocalNodeStateListenersTests, CanNotifyWithoutListener) {
		// Arrange:
		model::ChainScore score(0x12345670u, 0x89abcdefu);
		TestContext context;

		// Act:
		context.notifyScore(score);

		// Assert:
		EXPECT_EQ(0u, context.params().size());
	}

	TEST(LocalNodeStateListenersTests, CanNotifySubscribedListener) {
		// Arrange:
		model::ChainScore score(0x12345670u, 0x89abcdefu);
		TestContext context;
		context.subscribeDefault();

		// Act:
		context.notifyScore(score);

		// Assert:
		ASSERT_EQ(1u, context.params().size());
		EXPECT_EQ(score, context.params()[0].Score);
		EXPECT_TRUE(context.params()[0].IsMarkedCache);
	}

	namespace {
		auto GenerateScores(size_t numScores) {
			std::vector<model::ChainScore> scores;
			for (auto i = 0u; i < numScores; ++i)
				scores.push_back(model::ChainScore(i));

			return scores;
		}
	}

	TEST(LocalNodeStateListenersTests, CanNotifyMultipleTimes) {
		// Arrange:
		auto scores = GenerateScores(100u);
		TestContext context;
		context.subscribeDefault();

		// Act:
		for (const auto& score : scores)
			context.notifyScore(score);

		// Assert:
		ASSERT_EQ(scores.size(), context.params().size());
		auto i = 0u;
		for (const auto& score : scores) {
			EXPECT_EQ(score, context.params()[i].Score);
			EXPECT_TRUE(context.params()[i].IsMarkedCache);
			++i;
		}
	}

	TEST(LocalNodeStateListenersTests, SubscribeOverwritesPreviousListener) {
		// Arrange:
		auto scores = GenerateScores(100u);
		TestContext context;
		context.subscribeDefault();
		size_t numCalls = 0;
		context.subscribe([&numCalls](const auto&, const auto&) { ++numCalls; });

		// Act:
		for (const auto& score : scores)
			context.notifyScore(score);

		// Assert:
		EXPECT_EQ(0u, context.params().size());
		EXPECT_EQ(scores.size(), numCalls);
	}
}}}
