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

#include "mongo/src/ApiStateChangeSubscriber.h"
#include "catapult/model/ChainScore.h"
#include "tests/TestHarness.h"

namespace catapult { namespace mongo {

#define TEST_CLASS ApiStateChangeSubscriberTests

	namespace {
		// region basic mocks

		class MockChainScoreProvider : public ChainScoreProvider {
		public:
			const auto& capturedScores() const {
				return m_scores;
			}

		public:
			void saveScore(const model::ChainScore& chainScore) override {
				m_scores.push_back(chainScore);
			}

		private:
			std::vector<model::ChainScore> m_scores;
		};

		class MockExternalCacheStorage : public ExternalCacheStorage {
		public:
			MockExternalCacheStorage() : ExternalCacheStorage("MockExternalCacheStorage", std::numeric_limits<size_t>::max())
			{}

		public:
			const auto& capturedChanges() const {
				return m_capturedChanges;
			}

		public:
			void saveDelta(const cache::CacheChanges& changes) override {
				m_capturedChanges.push_back(&changes);
			}

		private:
			std::vector<const cache::CacheChanges*> m_capturedChanges;
		};

		// endregion

		// region test context

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

		// endregion
	}

	TEST(TEST_CLASS, NotifyScoreChangeForwardsToChainScoreProvider) {
		// Arrange:
		TestContext context;
		auto chainScore = model::ChainScore(123, 435);

		// Act:
		context.subscriber().notifyScoreChange(chainScore);

		// Assert:
		ASSERT_EQ(1u, context.chainScoreProvider().capturedScores().size());
		EXPECT_EQ(chainScore, context.chainScoreProvider().capturedScores()[0]);

		EXPECT_TRUE(context.externalCacheStorage().capturedChanges().empty());
	}

	TEST(TEST_CLASS, NotifyStateChangeForwardsToExternalCacheStorage) {
		// Arrange:
		TestContext context;
		auto stateChangeInfo = subscribers::StateChangeInfo(cache::CacheChanges({}), model::ChainScore::Delta(435), Height(123));

		// Act:
		context.subscriber().notifyStateChange(stateChangeInfo);

		// Assert:
		EXPECT_TRUE(context.chainScoreProvider().capturedScores().empty());

		ASSERT_EQ(1u, context.externalCacheStorage().capturedChanges().size());
		EXPECT_EQ(&stateChangeInfo.CacheChanges, context.externalCacheStorage().capturedChanges()[0]);
	}
}}
