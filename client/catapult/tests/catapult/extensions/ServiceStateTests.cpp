#include "catapult/extensions/ServiceState.h"
#include "catapult/cache/MemoryUtCache.h"
#include "catapult/extensions/LocalNodeChainScore.h"
#include "catapult/extensions/ServiceLocator.h"
#include "catapult/ionet/NodeContainer.h"
#include "catapult/thread/MultiServicePool.h"
#include "tests/test/core/mocks/MockMemoryBasedStorage.h"
#include "tests/test/local/LocalTestUtils.h"
#include "tests/test/other/mocks/MockNodeSubscriber.h"
#include "tests/test/other/mocks/MockStateChangeSubscriber.h"
#include "tests/test/other/mocks/MockTransactionStatusSubscriber.h"
#include "tests/TestHarness.h"

namespace catapult { namespace extensions {

#define TEST_CLASS ServiceStateTests

	TEST(TEST_CLASS, CanCreateServiceState) {
		// Arrange:
		auto config = test::CreateUninitializedLocalNodeConfiguration();
		ionet::NodeContainer nodes;
		auto catapultCache = cache::CatapultCache({});
		state::CatapultState catapultState;
		io::BlockStorageCache storage(std::make_unique<mocks::MockMemoryBasedStorage>());
		LocalNodeChainScore score;
		auto pUtCache = test::CreateUtCacheProxy();

		auto numTimeSupplierCalls = 0u;
		auto timeSupplier = [&numTimeSupplierCalls]() {
			++numTimeSupplierCalls;
			return Timestamp(111);
		};

		mocks::MockTransactionStatusSubscriber transactionStatusSubscriber;
		mocks::MockStateChangeSubscriber stateChangeSubscriber;
		mocks::MockNodeSubscriber nodeSubscriber;

		std::vector<utils::DiagnosticCounter> counters;
		plugins::PluginManager pluginManager(config.BlockChain);
		thread::MultiServicePool pool("test", 1);

		// Act:
		auto state = ServiceState(
				config,
				nodes,
				catapultCache,
				catapultState,
				storage,
				score,
				*pUtCache,
				timeSupplier,
				transactionStatusSubscriber,
				stateChangeSubscriber,
				nodeSubscriber,
				counters,
				pluginManager,
				pool);

		// Assert:
		// - check references
		EXPECT_EQ(&config, &state.config());
		EXPECT_EQ(&nodes, &state.nodes());
		EXPECT_EQ(&catapultCache, &state.cache());
		EXPECT_EQ(&catapultState, &state.state());
		EXPECT_EQ(&storage, &state.storage());
		EXPECT_EQ(&score, &state.score());
		EXPECT_EQ(pUtCache.get(), &state.utCache());

		EXPECT_EQ(&transactionStatusSubscriber, &state.transactionStatusSubscriber());
		EXPECT_EQ(&stateChangeSubscriber, &state.stateChangeSubscriber());
		EXPECT_EQ(&nodeSubscriber, &state.nodeSubscriber());

		EXPECT_EQ(&counters, &state.counters());
		EXPECT_EQ(&pluginManager, &state.pluginManager());
		EXPECT_EQ(&pool, &state.pool());

		// - check functions
		EXPECT_EQ(Timestamp(111), state.timeSupplier()());
		EXPECT_EQ(1u, numTimeSupplierCalls);

		// - check empty
		EXPECT_TRUE(state.tasks().empty());
		EXPECT_EQ(0u, state.packetHandlers().size());
		EXPECT_TRUE(state.hooks().chainSyncedPredicate()); // just check that hooks is valid and default predicate can be called
		EXPECT_TRUE(state.packetIoPickers().pickMatching(utils::TimeSpan::FromSeconds(1), ionet::NodeRoles::None).empty());
	}
}}
