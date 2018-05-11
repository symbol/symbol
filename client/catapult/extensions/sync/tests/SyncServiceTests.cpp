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

#include "sync/src/SyncService.h"
#include "catapult/extensions/ServerHooks.h"
#include "tests/test/core/TransactionInfoTestUtils.h"
#include "tests/test/local/ServiceLocatorTestContext.h"
#include "tests/test/local/ServiceTestUtils.h"
#include "tests/test/net/mocks/MockPacketWriters.h"
#include "tests/TestHarness.h"

namespace catapult { namespace sync {

#define TEST_CLASS SyncServiceTests

	namespace {
		constexpr auto Num_Expected_Tasks = 3u;

		struct SyncServiceTraits {
			static constexpr auto CreateRegistrar = CreateSyncServiceRegistrar;
		};

		class TestContext : public test::ServiceLocatorTestContext<SyncServiceTraits> {
		public:
			TestContext() {
				// register dependent service
				locator().registerService("writers", std::make_shared<mocks::MockPacketWriters>());

				// set up hooks
				auto& hooks = testState().state().hooks();
				hooks.setCompletionAwareBlockRangeConsumerFactory([](auto) {
					return [](auto&&, auto) { return disruptor::DisruptorElementId(); };
				});
				hooks.setTransactionRangeConsumerFactory([](auto) { return [](auto&&) {}; });
			}
		};
	}

	ADD_SERVICE_REGISTRAR_INFO_TEST(Sync, Post_Range_Consumers)

	// region tasks

	TEST(TEST_CLASS, ConnectPeersTaskIsScheduled) {
		// Assert:
		test::AssertRegisteredTask(TestContext(), Num_Expected_Tasks, "connect peers task for service Sync");
	}

	TEST(TEST_CLASS, SynchronizerTaskIsScheduled) {
		// Assert:
		test::AssertRegisteredTask(TestContext(), Num_Expected_Tasks, "synchronizer task");
	}

	TEST(TEST_CLASS, PullUtTaskIsScheduled) {
		// Assert:
		test::AssertRegisteredTask(TestContext(), Num_Expected_Tasks, "pull unconfirmed transactions task");
	}

	// endregion
}}
