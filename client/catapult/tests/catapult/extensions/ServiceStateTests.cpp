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

#include "catapult/extensions/ServiceState.h"
#include "catapult/cache_tx/MemoryUtCache.h"
#include "catapult/extensions/LocalNodeChainScore.h"
#include "catapult/extensions/ServiceLocator.h"
#include "catapult/ionet/NodeContainer.h"
#include "catapult/thread/MultiServicePool.h"
#include "tests/test/core/mocks/MockMemoryBlockStorage.h"
#include "tests/test/local/LocalTestUtils.h"
#include "tests/test/other/mocks/MockNodeSubscriber.h"
#include "tests/test/other/mocks/MockStateChangeSubscriber.h"
#include "tests/test/other/mocks/MockTransactionStatusSubscriber.h"
#include "tests/test/plugins/PluginManagerFactory.h"
#include "tests/TestHarness.h"

namespace catapult { namespace extensions {

#define TEST_CLASS ServiceStateTests

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

		mocks::MockTransactionStatusSubscriber transactionStatusSubscriber;
		mocks::MockStateChangeSubscriber stateChangeSubscriber;
		mocks::MockNodeSubscriber nodeSubscriber;

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
				pUtCache->get(),
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
		EXPECT_EQ(&storage, &state.storage());
		EXPECT_EQ(&score, &state.score());

		EXPECT_EQ(&pUtCache->get(), &const_cast<const ServiceState&>(state).utCache());
		EXPECT_EQ(&pUtCache->get(), &state.utCache());

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
		EXPECT_EQ(1234u * 1024, state.packetHandlers().maxPacketDataSize()); // should be initialized from config

		EXPECT_TRUE(state.hooks().chainSyncedPredicate()); // just check that hooks is valid and default predicate can be called
		EXPECT_TRUE(state.packetIoPickers().pickMatching(utils::TimeSpan::FromSeconds(1), ionet::NodeRoles::None).empty());
	}
}}
