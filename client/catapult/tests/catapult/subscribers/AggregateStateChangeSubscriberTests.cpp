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

#include "catapult/subscribers/AggregateStateChangeSubscriber.h"
#include "catapult/subscribers/StateChangeInfo.h"
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
			std::vector<const StateChangeInfo*> StateChangeInfos;

		public:
			void notifyStateChange(const StateChangeInfo& info) override {
				StateChangeInfos.push_back(&info);
			}
		};

		TestContext<MockStateChangeSubscriber> context;

		auto cache = test::CreateEmptyCatapultCache();
		auto cacheDelta = cache.createDelta();
		StateChangeInfo stateChangeInfo{ cache::CacheChanges(cacheDelta), model::ChainScore::Delta(), Height(444) };

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
