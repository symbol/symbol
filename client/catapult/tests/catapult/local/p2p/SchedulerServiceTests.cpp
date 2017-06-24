#include "catapult/local/p2p/SchedulerService.h"
#include "catapult/cache/MemoryUtCache.h"
#include "catapult/chain/UnlockedAccounts.h"
#include "catapult/plugins/PluginManager.h"
#include "tests/test/local/LocalNodeTestState.h"
#include "tests/test/local/LocalTestUtils.h"
#include "tests/test/local/NetworkTestUtils.h"
#include "tests/test/local/SchedulerServiceTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace local { namespace p2p {

	namespace {
		using ServiceType = SchedulerService;

		class TestContext {
		public:
			TestContext()
					: m_pluginManager(model::BlockChainConfiguration::Uninitialized())
					, m_pUnconfirmedTransactionsCache(test::CreateUnconfirmedTransactionsCache())
					, m_networkHeight(0)
					, m_unlockedAccounts(7)
					, m_service(
							m_state.cref(),
							m_pluginManager,
							*m_pUnconfirmedTransactionsCache,
							m_networkHeight,
							[](auto) { return [](const auto&, const auto&) { return 0u; }; },
							[](const auto&) {})
					, m_pool(2, "scheduler")
			{}

		public:
			auto& pool() {
				return m_pool;
			}

			auto& service() {
				return m_service;
			}

			void boot() {
				m_service.boot(m_pool, m_packetWritersHolder.get(), m_unlockedAccounts);
			}

		private:
			test::LocalNodeTestState m_state;
			plugins::PluginManager m_pluginManager;
			std::unique_ptr<cache::MemoryUtCache> m_pUnconfirmedTransactionsCache;
			NetworkChainHeight m_networkHeight;

			test::DefaultPacketWritersHolder m_packetWritersHolder;
			chain::UnlockedAccounts m_unlockedAccounts;

			ServiceType m_service;
			thread::MultiServicePool m_pool;
		};
	}

	// region boot / shutdown

	TEST(SchedulerServiceTests, CanBootService) {
		// Assert:
		test::AssertCanBootSchedulerService<TestContext>(5);
	}

	TEST(SchedulerServiceTests, CanShutdownService) {
		// Assert:
		test::AssertCanShutdownSchedulerService<TestContext>();
	}

	// endregion
}}}
