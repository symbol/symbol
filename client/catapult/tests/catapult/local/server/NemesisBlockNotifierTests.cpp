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

#include "catapult/local/server/NemesisBlockNotifier.h"
#include "catapult/cache_core/AccountStateCache.h"
#include "catapult/model/Address.h"
#include "catapult/plugins/PluginManager.h"
#include "tests/test/core/BlockStatementTestUtils.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/test/core/mocks/MockMemoryBlockStorage.h"
#include "tests/test/local/LocalTestUtils.h"
#include "tests/test/nemesis/NemesisCompatibleConfiguration.h"
#include "tests/test/nemesis/NemesisTestUtils.h"
#include "tests/test/nodeps/TestNetworkConstants.h"
#include "tests/test/other/mocks/MockBlockChangeSubscriber.h"
#include "tests/test/other/mocks/MockFinalizationSubscriber.h"
#include "tests/test/other/mocks/MockStateChangeSubscriber.h"
#include "tests/TestHarness.h"

namespace catapult { namespace local {

#define TEST_CLASS NemesisBlockNotifierTests

	namespace {
		// region TestContext

		class TestContext {
		public:
			explicit TestContext(uint32_t numBlocks)
					: m_pPluginManager(test::CreatePluginManagerWithRealPlugins(CreateBlockChainConfiguration()))
					, m_cache(m_pPluginManager->createCache())
					, m_pStorage(mocks::CreateMemoryBlockStorageCache(numBlocks))
					, m_notifier(m_pPluginManager->config(), m_cache, *m_pStorage, *m_pPluginManager)
			{}

		public:
			auto& notifier() {
				return m_notifier;
			}

		public:
			auto nemesisEntityHash() {
				return m_pStorage->view().loadBlockElement(Height(1))->EntityHash;
			}

		public:
			void addRandomAccountToCache() {
				auto cacheDelta = m_cache.createDelta();
				cacheDelta.sub<cache::AccountStateCache>().addAccount(test::GenerateRandomByteArray<Key>(), Height(1));
				m_cache.commit(Height());
			}

			auto reseedNemesisWithStatement() {
				auto modifier = m_pStorage->modifier();
				modifier.dropBlocksAfter(Height(0));

				model::Block block;
				block.Size = sizeof(model::BlockHeader);
				block.Height = Height(1);
				auto blockElement = test::BlockToBlockElement(block);
				blockElement.OptionalStatement = test::GenerateRandomStatements({ 3, 1, 2 });
				modifier.saveBlock(blockElement);
				modifier.commit();

				return blockElement.OptionalStatement;
			}

		private:
			static model::BlockChainConfiguration CreateBlockChainConfiguration() {
				return test::CreateCatapultConfigurationWithNemesisPluginExtensions("").BlockChain;
			}

		private:
			std::shared_ptr<plugins::PluginManager> m_pPluginManager;
			cache::CatapultCache m_cache;
			std::unique_ptr<io::BlockStorageCache> m_pStorage;
			NemesisBlockNotifier m_notifier;
		};

		// endregion
	}

	// region block change notifications

	TEST(TEST_CLASS, BlockChangeNotificationsAreNotRaisedWhenHeightIsGreaterThanOne) {
		// Arrange:
		TestContext context(2);
		mocks::MockBlockChangeSubscriber subscriber;

		// Act:
		EXPECT_THROW(context.notifier().raise(subscriber), catapult_runtime_error);

		// Assert:
		const auto& capturedBlockElements = subscriber.copiedBlockElements();
		EXPECT_EQ(0u, capturedBlockElements.size());
	}

	TEST(TEST_CLASS, BlockChangeNotificationsAreNotRaisedWhenHeightIsEqualToOneAndPreviousExecutionIsDetected) {
		// Arrange: add account to indicate previous execution
		TestContext context(1);
		context.addRandomAccountToCache();
		mocks::MockBlockChangeSubscriber subscriber;

		// Act:
		EXPECT_THROW(context.notifier().raise(subscriber), catapult_runtime_error);

		// Assert:
		const auto& capturedBlockElements = subscriber.copiedBlockElements();
		EXPECT_EQ(0u, capturedBlockElements.size());
	}

	TEST(TEST_CLASS, BlockChangeNotificationsAreRaisedWhenHeightIsEqualToOneAndPreviousExecutionIsNotDetected_WithoutStatement) {
		// Arrange:
		TestContext context(1);
		mocks::MockBlockChangeSubscriber subscriber;

		// Act:
		context.notifier().raise(subscriber);

		// Assert:
		const auto& capturedBlockElements = subscriber.copiedBlockElements();
		ASSERT_EQ(1u, capturedBlockElements.size());
		EXPECT_EQ(Height(1), capturedBlockElements[0]->Block.Height);

		EXPECT_FALSE(capturedBlockElements[0]->OptionalStatement);
	}

	TEST(TEST_CLASS, BlockChangeNotificationsAreRaisedWhenHeightIsEqualToOneAndPreviousExecutionIsNotDetected_WithStatement) {
		// Arrange:
		TestContext context(1);
		auto pBlockStatement = context.reseedNemesisWithStatement();
		mocks::MockBlockChangeSubscriber subscriber;

		// Act:
		context.notifier().raise(subscriber);

		// Assert:
		const auto& capturedBlockElements = subscriber.copiedBlockElements();
		ASSERT_EQ(1u, capturedBlockElements.size());
		EXPECT_EQ(Height(1), capturedBlockElements[0]->Block.Height);

		ASSERT_TRUE(capturedBlockElements[0]->OptionalStatement);
		test::AssertEqual(*pBlockStatement, *capturedBlockElements[0]->OptionalStatement);
	}

	// endregion

	// region finalization notifications

	TEST(TEST_CLASS, FinalizationNotificationsAreNotRaisedWhenHeightIsGreaterThanOne) {
		// Arrange:
		TestContext context(2);
		mocks::MockFinalizationSubscriber subscriber;

		// Act:
		EXPECT_THROW(context.notifier().raise(subscriber), catapult_runtime_error);

		// Assert:
		EXPECT_EQ(0u, subscriber.finalizedBlockParams().params().size());
	}

	TEST(TEST_CLASS, FinalizationNotificationsAreRaisedWhenHeightIsEqualToOne) {
		// Arrange:
		TestContext context(1);
		mocks::MockFinalizationSubscriber subscriber;

		// Act:
		context.notifier().raise(subscriber);

		// Assert:
		ASSERT_EQ(1u, subscriber.finalizedBlockParams().params().size());

		const auto& subscriberParams = subscriber.finalizedBlockParams().params()[0];
		EXPECT_EQ(model::FinalizationRound({ FinalizationEpoch(1), FinalizationPoint(1) }), subscriberParams.Round);
		EXPECT_EQ(Height(1), subscriberParams.Height);
		EXPECT_EQ(context.nemesisEntityHash(), subscriberParams.Hash);
	}

	// endregion

	// region state change notifications

	namespace {
		model::AddressSet GetAddedAccountAddresses(const cache::CacheChanges& cacheChanges) {
			model::AddressSet addresses;
			auto accountStateCacheChanges = cacheChanges.sub<cache::AccountStateCache>();
			for (const auto* pAccountState : accountStateCacheChanges.addedElements())
				addresses.insert(pAccountState->Address);

			return addresses;
		}

		bool ContainsAddress(const model::AddressSet& addresses, const Address& address) {
			return addresses.cend() != addresses.find(address);
		}

		bool ContainsModifiedPrivate(const model::AddressSet& addresses, const char* privateKeyString) {
			return ContainsAddress(addresses, test::RawPrivateKeyToAddress(privateKeyString));
		}
	}

	TEST(TEST_CLASS, StateChangeNotificationsAreNotRaisedWhenHeightIsGreaterThanOne) {
		// Arrange:
		TestContext context(2);
		mocks::MockStateChangeSubscriber subscriber;

		// Act:
		EXPECT_THROW(context.notifier().raise(subscriber), catapult_runtime_error);

		// Assert:
		EXPECT_EQ(0u, subscriber.numScoreChanges());
		EXPECT_EQ(0u, subscriber.numStateChanges());
	}

	TEST(TEST_CLASS, StateChangeNotificationsAreRaisedWhenHeightIsEqualToOne) {
		// Arrange:
		TestContext context(1);
		mocks::MockStateChangeSubscriber subscriber;

		// - register consumer because CatapultCacheDelta wrapped by CacheChanges is temporary and will be out of scope below
		model::AddressSet addedAddresses;
		subscriber.setCacheChangesConsumer([&addedAddresses](const auto& cacheChanges) {
			addedAddresses = GetAddedAccountAddresses(cacheChanges);
		});

		// Act:
		context.notifier().raise(subscriber);

		// Assert:
		ASSERT_EQ(1u, subscriber.numScoreChanges());
		ASSERT_EQ(1u, subscriber.numStateChanges());

		const auto& chainScore = subscriber.lastChainScore();
		EXPECT_EQ(model::ChainScore(1), chainScore);

		const auto& stateChangeInfo = subscriber.lastStateChangeInfo();
		EXPECT_EQ(model::ChainScore(1), stateChangeInfo.ScoreDelta);
		EXPECT_EQ(Height(1), stateChangeInfo.Height);

		// - check account state changes
		EXPECT_EQ(3u + CountOf(test::Test_Network_Private_Keys), addedAddresses.size());

		// - check nemesis and rental fee sinks
		EXPECT_TRUE(ContainsModifiedPrivate(addedAddresses, test::Test_Network_Nemesis_Private_Key));
		EXPECT_TRUE(ContainsAddress(addedAddresses, model::StringToAddress(test::Namespace_Rental_Fee_Sink_Address)));
		EXPECT_TRUE(ContainsAddress(addedAddresses, model::StringToAddress(test::Mosaic_Rental_Fee_Sink_Address)));

		// - check recipient accounts
		for (const auto* pRecipientPrivateKeyString : test::Test_Network_Private_Keys)
			EXPECT_TRUE(ContainsModifiedPrivate(addedAddresses, pRecipientPrivateKeyString)) << pRecipientPrivateKeyString;
	}

	// endregion
}}
