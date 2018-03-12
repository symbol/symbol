#include "catapult/subscribers/AggregateStateChangeSubscriber.h"
#include "catapult/consumers/StateChangeInfo.h"
#include "catapult/model/ChainScore.h"
#include "tests/catapult/subscribers/test/AggregateSubscriberTestContext.h"
#include "tests/catapult/subscribers/test/UnsupportedSubscribers.h"
#include "tests/test/cache/CacheTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace subscribers {

#define TEST_CLASS AggregateStateChangeSubscriberTests

	namespace {
		using UnsupportedStateChangeSubscriber = test::UnsupportedStateChangeSubscriber;

		template<typename TStateChangeSubscriber>
		using TestContext = test::AggregateSubscriberTestContext<
				TStateChangeSubscriber,
				AggregateStateChangeSubscriber<TStateChangeSubscriber>>;
	}

	TEST(TEST_CLASS, NotifyScoreChangeForwardsToAllSubscribers) {
		// Arrange:
		class MockStateChangeSubscriber : public UnsupportedStateChangeSubscriber {
		public:
			std::vector<const model::ChainScore*> Scores;

		public:
			void notifyScoreChange(const model::ChainScore& chainScore) override {
				Scores.push_back(&chainScore);
			}
		};

		TestContext<MockStateChangeSubscriber> context;
		model::ChainScore chainScore;

		// Sanity:
		EXPECT_EQ(3u, context.subscribers().size());

		// Act:
		context.aggregate().notifyScoreChange(chainScore);

		// Assert:
		auto i = 0u;
		for (const auto* pSubscriber : context.subscribers()) {
			auto message = "subscriber at " + std::to_string(i++);
			ASSERT_EQ(1u, pSubscriber->Scores.size()) << message;
			EXPECT_EQ(&chainScore, pSubscriber->Scores[0]) << message;
		}
	}

	TEST(TEST_CLASS, NotifyStateChangeForwardsToAllSubscribers) {
		// Arrange:
		class MockStateChangeSubscriber : public UnsupportedStateChangeSubscriber {
		public:
			std::vector<const consumers::StateChangeInfo*> StateChangeInfos;

		public:
			void notifyStateChange(const consumers::StateChangeInfo& info) override {
				StateChangeInfos.push_back(&info);
			}
		};

		TestContext<MockStateChangeSubscriber> context;

		auto cache = test::CreateEmptyCatapultCache();
		auto cacheDelta = cache.createDelta();
		model::ChainScore scoreDelta;
		consumers::StateChangeInfo stateChangeInfo(cacheDelta, scoreDelta, Height(444));

		// Sanity:
		EXPECT_EQ(3u, context.subscribers().size());

		// Act:
		context.aggregate().notifyStateChange(stateChangeInfo);

		// Assert:
		auto i = 0u;
		for (const auto* pSubscriber : context.subscribers()) {
			auto message = "subscriber at " + std::to_string(i++);
			ASSERT_EQ(1u, pSubscriber->StateChangeInfos.size()) << message;
			EXPECT_EQ(&stateChangeInfo, pSubscriber->StateChangeInfos[0]) << message;
		}
	}
}}
