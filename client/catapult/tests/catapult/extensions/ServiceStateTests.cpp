/**
*** Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
*** Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
*** All rights reserved.
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

#include "catapult/extensions/ServiceState.h"
#include "catapult/cache_tx/MemoryUtCache.h"
#include "catapult/extensions/LocalNodeChainScore.h"
#include "catapult/extensions/PeersConnectionTasks.h"
#include "catapult/extensions/ServiceLocator.h"
#include "catapult/ionet/NodeContainer.h"
#include "catapult/thread/MultiServicePool.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/test/core/mocks/MockMemoryBlockStorage.h"
#include "tests/test/local/LocalTestUtils.h"
#include "tests/test/local/ServiceLocatorTestContext.h"
#include "tests/test/other/mocks/MockFinalizationSubscriber.h"
#include "tests/test/other/mocks/MockNodeSubscriber.h"
#include "tests/test/other/mocks/MockStateChangeSubscriber.h"
#include "tests/test/other/mocks/MockTransactionStatusSubscriber.h"
#include "tests/test/plugins/PluginManagerFactory.h"
#include "tests/TestHarness.h"

namespace catapult { namespace extensions {

#define TEST_CLASS ServiceStateTests

	// region ServiceState

	TEST(TEST_CLASS, CanCreateServiceState) {
		// Arrange:
		auto config = test::CreateUninitializedCatapultConfiguration();
		const_cast<utils::FileSize&>(config.Node.MaxPacketDataSize) = utils::FileSize::FromKilobytes(1234);

		ionet::NodeContainer nodes;
		auto catapultCache = cache::CatapultCache({});
		io::BlockStorageCache storage(
				std::make_unique<mocks::MockMemoryBlockStorage>(),
				std::make_unique<mocks::MockMemoryBlockStorage>());
		LocalNodeChainScore score;
		auto pUtCache = test::CreateUtCacheProxy();

		auto numTimeSupplierCalls = 0u;
		auto timeSupplier = [&numTimeSupplierCalls]() {
			++numTimeSupplierCalls;
			return Timestamp(111);
		};

		mocks::MockFinalizationSubscriber finalizationSubscriber;
		mocks::MockNodeSubscriber nodeSubscriber;
		mocks::MockStateChangeSubscriber stateChangeSubscriber;
		mocks::MockTransactionStatusSubscriber transactionStatusSubscriber;

		std::vector<utils::DiagnosticCounter> counters;
		auto pluginManager = test::CreatePluginManager(config.BlockChain);
		thread::MultiServicePool pool("test", 1);

		// Act:
		auto state = ServiceState(
				config,
				nodes,
				catapultCache,
				storage,
				score,
				*pUtCache,
				timeSupplier,
				finalizationSubscriber,
				nodeSubscriber,
				stateChangeSubscriber,
				transactionStatusSubscriber,
				counters,
				pluginManager,
				pool);

		// Assert:
		// - check references
		EXPECT_EQ(&config, &state.config());
		EXPECT_EQ(&nodes, &state.nodes());
		EXPECT_EQ(&catapultCache, &state.cache());
		EXPECT_EQ(&storage, &state.storage());
		EXPECT_EQ(&score, &state.score());

		EXPECT_EQ(&pUtCache->get(), &const_cast<const ServiceState&>(state).utCache());
		EXPECT_EQ(&pUtCache->get(), &state.utCache());

		EXPECT_EQ(&finalizationSubscriber, &state.finalizationSubscriber());
		EXPECT_EQ(&nodeSubscriber, &state.nodeSubscriber());
		EXPECT_EQ(&stateChangeSubscriber, &state.stateChangeSubscriber());
		EXPECT_EQ(&transactionStatusSubscriber, &state.transactionStatusSubscriber());

		EXPECT_EQ(&counters, &state.counters());
		EXPECT_EQ(&pluginManager, &state.pluginManager());
		EXPECT_EQ(&pool, &state.pool());

		// - check functions
		EXPECT_EQ(Timestamp(111), state.timeSupplier()());
		EXPECT_EQ(1u, numTimeSupplierCalls);

		// - check empty
		EXPECT_TRUE(state.tasks().empty());

		EXPECT_EQ(0u, state.packetHandlers().size());
		EXPECT_EQ(1234u * 1024, state.packetHandlers().maxPacketDataSize()); // should be initialized from config

		EXPECT_TRUE(state.hooks().chainSyncedPredicate()); // just check that hooks is valid and default predicate can be called
		EXPECT_TRUE(state.packetIoPickers().pickMatching(utils::TimeSpan::FromSeconds(1), ionet::NodeRoles::None).empty());
	}

	// endregion

	// region finalized height / height hash pair suppliers

	namespace {
		struct LocalFinalizedHeightHashPairSupplierTraits {
			static constexpr auto CreateSupplier = CreateLocalFinalizedHeightHashPairSupplier;

			static void SetSupplier(ServerHooks& hooks, const model::HeightHashPair& finalizedHeightHashPair) {
				hooks.setLocalFinalizedHeightHashPairSupplier([finalizedHeightHashPair]() { return finalizedHeightHashPair; });
			}

			static model::HeightHashPair GetExpected(Height height, const Hash256& hash) {
				return { height, hash };
			}
		};

		struct LocalFinalizedHeightSupplierTraits {
			static constexpr auto CreateSupplier = CreateLocalFinalizedHeightSupplier;
			static constexpr auto SetSupplier = LocalFinalizedHeightHashPairSupplierTraits::SetSupplier;

			static Height GetExpected(Height height, const Hash256&) {
				return height;
			}
		};

		struct NetworkFinalizedHeightHashPairSupplierTraits {
			static constexpr auto CreateSupplier = CreateNetworkFinalizedHeightHashPairSupplier;

			static void SetSupplier(ServerHooks& hooks, const model::HeightHashPair& finalizedHeightHashPair) {
				hooks.setNetworkFinalizedHeightHashPairSupplier([finalizedHeightHashPair]() { return finalizedHeightHashPair; });
			}

			static constexpr auto GetExpected = LocalFinalizedHeightHashPairSupplierTraits::GetExpected;
		};
	}

#define HEIGHT_HASH_PAIR_SUPPLIER_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, CreateLocalFinalizedHeightHashPairSupplier_##TEST_NAME) { \
		TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<LocalFinalizedHeightHashPairSupplierTraits>(); \
	} \
	TEST(TEST_CLASS, CreateLocalFinalizedHeightSupplier_##TEST_NAME) { \
		TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<LocalFinalizedHeightSupplierTraits>(); \
	} \
	TEST(TEST_CLASS, CreateNetworkFinalizedHeightHashPairSupplier_##TEST_NAME) { \
		TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<NetworkFinalizedHeightHashPairSupplierTraits>(); \
	} \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	namespace {
		template<typename TTraits>
		void AssertLocalFinalizedHeightHashPairSupplier(uint32_t maxRollbackBlocks, uint32_t numBlocks, Height expectedHeight) {
			// Arrange:
			test::ServiceTestState testState;

			auto lastFinalizedHash = test::GenerateRandomByteArray<Hash256>();
			const_cast<uint32_t&>(testState.state().config().BlockChain.MaxRollbackBlocks) = maxRollbackBlocks;

			mocks::SeedStorageWithFixedSizeBlocks(testState.state().storage(), numBlocks);
			TTraits::SetSupplier(testState.state().hooks(), { Height(7), lastFinalizedHash });

			auto expectedHash = 0 == maxRollbackBlocks
					? lastFinalizedHash
					: testState.state().storage().view().loadBlockElement(expectedHeight)->EntityHash;

			// Act:
			auto supplier = TTraits::CreateSupplier(testState.state());

			// Assert:
			EXPECT_EQ(TTraits::GetExpected(expectedHeight, expectedHash), supplier())
					<< "maxRollbackBlocks = " << maxRollbackBlocks << ", numBlocks = " << numBlocks;
		}
	}

	HEIGHT_HASH_PAIR_SUPPLIER_TEST(ReturnsRegisteredSupplierWhenMaxRollbackBlocksIsZero) {
		AssertLocalFinalizedHeightHashPairSupplier<TTraits>(0, 4, Height(7));
		AssertLocalFinalizedHeightHashPairSupplier<TTraits>(0, 20, Height(7));
	}

	HEIGHT_HASH_PAIR_SUPPLIER_TEST(ReturnsStorageBasedSupplierWhenMaxRollbackBlocksIsNonzero) {
		AssertLocalFinalizedHeightHashPairSupplier<TTraits>(5, 4, Height(1));
		AssertLocalFinalizedHeightHashPairSupplier<TTraits>(5, 5, Height(1));
		AssertLocalFinalizedHeightHashPairSupplier<TTraits>(5, 6, Height(1));
		AssertLocalFinalizedHeightHashPairSupplier<TTraits>(5, 7, Height(2));
		AssertLocalFinalizedHeightHashPairSupplier<TTraits>(5, 20, Height(15));
	}

	// endregion

	// region CreateOutgoingSelectorSettings

	TEST(TEST_CLASS, CanCreateOutgoingSelectorSettings) {
		// Arrange:
		test::ServiceTestState testState;
		const_cast<uint16_t&>(testState.state().config().Node.OutgoingConnections.MaxConnections) = 17;
		const_cast<uint16_t&>(testState.state().config().Node.IncomingConnections.MaxConnections) = 14;

		// Act:
		auto settings = CreateOutgoingSelectorSettings(testState.state(), ionet::ServiceIdentifier(123), ionet::NodeRoles::Api);

		// Assert:
		EXPECT_EQ(&testState.state().nodes(), &settings.Nodes);
		EXPECT_EQ(ionet::ServiceIdentifier(123), settings.ServiceId);
		EXPECT_EQ(ionet::IpProtocol::IPv4, settings.SupportedProtocols);
		EXPECT_EQ(ionet::NodeRoles::Api, settings.RequiredRole);
		EXPECT_EQ(17u, settings.Config.MaxConnections);
		EXPECT_TRUE(!!settings.ImportanceRetriever);
	}

	// endregion

	// region CreateIncomingSelectorSettings

	TEST(TEST_CLASS, CanCreateIncomingSelectorSettings) {
		// Arrange:
		test::ServiceTestState testState;
		const_cast<uint16_t&>(testState.state().config().Node.OutgoingConnections.MaxConnections) = 17;
		const_cast<uint16_t&>(testState.state().config().Node.IncomingConnections.MaxConnections) = 14;

		// Act:
		auto settings = CreateIncomingSelectorSettings(testState.state(), ionet::ServiceIdentifier(123));

		// Assert:
		EXPECT_EQ(&testState.state().nodes(), &settings.Nodes);
		EXPECT_EQ(ionet::ServiceIdentifier(123), settings.ServiceId);
		EXPECT_EQ(ionet::IpProtocol::All, settings.SupportedProtocols);
		EXPECT_EQ(ionet::NodeRoles::None, settings.RequiredRole);
		EXPECT_EQ(14u, settings.Config.MaxConnections);
		EXPECT_TRUE(!!settings.ImportanceRetriever);
	}

	// endregion

	// region CreateShouldProcessTransactionsPredicate

	namespace {
		bool RunTransactionPullPredicateTest(Timestamp networkTime, Timestamp lastBlockTime, const utils::TimeSpan& maxTimeBehind) {
			// Arrange:
			test::ServiceTestState testState(cache::CatapultCache({}), [networkTime]() { return networkTime; });
			const_cast<utils::TimeSpan&>(testState.state().config().Node.MaxTimeBehindPullTransactionsStart) = maxTimeBehind;

			// - storage already contains nemesis block (height 1)
			{
				auto modifier = testState.state().storage().modifier();
				auto pBlockTwo = test::GenerateBlockWithTransactions(0, Height(2), lastBlockTime);
				modifier.saveBlock(test::BlockToBlockElement(*pBlockTwo));
				modifier.commit();
			}

			// Act:
			auto predicate = CreateShouldProcessTransactionsPredicate(testState.state());
			return predicate();
		}
	}

	TEST(TEST_CLASS, CanCreateShouldProcessTransactionsPredicate) {
		// Assert: network <  last block
		EXPECT_TRUE(RunTransactionPullPredicateTest(Timestamp(30 * 1000 - 1), Timestamp(60 * 1000), utils::TimeSpan::FromSeconds(30)));
		EXPECT_TRUE(RunTransactionPullPredicateTest(Timestamp(30 * 1000), Timestamp(60 * 1000), utils::TimeSpan::FromSeconds(30)));
		EXPECT_TRUE(RunTransactionPullPredicateTest(Timestamp(60 * 1000 - 1), Timestamp(60 * 1000), utils::TimeSpan::FromSeconds(30)));

		// network ==  last block
		EXPECT_TRUE(RunTransactionPullPredicateTest(Timestamp(60 * 1000), Timestamp(60 * 1000), utils::TimeSpan::FromSeconds(30)));

		// network >  last block
		EXPECT_TRUE(RunTransactionPullPredicateTest(Timestamp(60 * 1000 + 1), Timestamp(60 * 1000), utils::TimeSpan::FromSeconds(30)));
		EXPECT_TRUE(RunTransactionPullPredicateTest(Timestamp(90 * 1000), Timestamp(60 * 1000), utils::TimeSpan::FromSeconds(30)));
		EXPECT_FALSE(RunTransactionPullPredicateTest(Timestamp(90 * 1000 + 1), Timestamp(60 * 1000), utils::TimeSpan::FromSeconds(30)));
	}

	// endregion
}}
