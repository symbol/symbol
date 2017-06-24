#include "catapult/local/api/SchedulerService.h"
#include "catapult/cache/MemoryUtCache.h"
#include "catapult/model/TransactionPlugin.h"
#include "tests/test/local/LocalNodeTestState.h"
#include "tests/test/local/LocalTestUtils.h"
#include "tests/test/local/NetworkTestUtils.h"
#include "tests/test/local/SchedulerServiceTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace local { namespace api {

	namespace {
		using ServiceType = SchedulerService;

		class TestContext {
		public:
			TestContext()
					: m_pTransactionRegistry(std::make_unique<model::TransactionRegistry>())
					, m_pUnconfirmedTransactionsCache(test::CreateUnconfirmedTransactionsCache())
					, m_service(
							m_state.cref(),
							*m_pTransactionRegistry,
							test::CreateViewProvider(*m_pUnconfirmedTransactionsCache),
							[](auto) { return [](const auto&, const auto&) { return 0u; }; })
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
				m_service.boot(m_pool, m_packetWritersHolder.get());
			}

		private:
			test::LocalNodeTestState m_state;
			std::unique_ptr<model::TransactionRegistry> m_pTransactionRegistry;
			std::unique_ptr<cache::MemoryUtCache> m_pUnconfirmedTransactionsCache;

			test::DefaultPacketWritersHolder m_packetWritersHolder;

			ServiceType m_service;
			thread::MultiServicePool m_pool;
		};
	}

	// region boot / shutdown

	TEST(SchedulerServiceTests, CanBootService) {
		// Assert:
		test::AssertCanBootSchedulerService<TestContext>(2);
	}

	TEST(SchedulerServiceTests, CanShutdownService) {
		// Assert:
		test::AssertCanShutdownSchedulerService<TestContext>();
	}

	// endregion
}}}
