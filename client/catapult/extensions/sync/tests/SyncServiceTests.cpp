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
