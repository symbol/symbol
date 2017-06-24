#include "catapult/local/p2p/DispatcherService.h"
#include "catapult/cache/AccountStateCache.h"
#include "catapult/cache/MemoryUtCache.h"
#include "tests/catapult/local/p2p/utils/PeerCacheTestUtils.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/test/core/mocks/MemoryBasedStorage.h"
#include "tests/test/core/mocks/MockTransaction.h"
#include "tests/test/local/DispatcherServiceTestUtils.h"
#include "tests/test/local/EntityFactory.h"
#include "tests/test/local/LocalNodeTestState.h"
#include "tests/test/local/LocalTestUtils.h"
#include "tests/test/nodeps/MijinConstants.h"
#include "tests/TestHarness.h"

namespace catapult { namespace local { namespace p2p {

	namespace {
		using ServiceType = DispatcherService;

		crypto::KeyPair GetNemesisAccountKeyPair() {
			return crypto::KeyPair::FromString(test::Mijin_Test_Private_Keys[0]); // use a nemesis account
		}

		class TestContext {
		public:
			TestContext()
					: m_signerKeyPair(GetNemesisAccountKeyPair()) // use a nemesis account
					, m_state(test::CreateCatapultCacheForDispatcherTests<test::PeerCacheFactory>())
					, m_pPluginManager(test::CreateDefaultPluginManager())
					, m_pUnconfirmedTransactionsCache(test::CreateUnconfirmedTransactionsCache())
					, m_numNewBlockElements(0)
					, m_numNewTransactionElements(0)
					, m_service(
							m_state.ref(),
							*m_pPluginManager,
							*m_pUnconfirmedTransactionsCache,
							HashPredicateFactory(*m_pUnconfirmedTransactionsCache, m_state.ref().Cache),
							[this](auto&&) {
								CATAPULT_LOG(debug) << "service forwarded new block element";
								++m_numNewBlockElements;
							},
							[this](auto&&) {
								CATAPULT_LOG(debug) << "service forwarded new transaction element";
								++m_numNewTransactionElements;
							})
					, m_pool(2, "dispatcher") {
				test::InitializeCatapultCacheForDispatcherTests(m_state.ref().Cache, m_signerKeyPair);

				// the service needs to be able to raise notifications from the mock transactions sent to it
				m_pPluginManager->addTransactionSupport(mocks::CreateMockTransactionPlugin());
			}

		public:
			auto& pool() {
				return m_pool;
			}

			auto& service() {
				return m_service;
			}

			void boot() {
				m_service.boot(m_pool);
			}

		public:
			size_t numNewBlockElements() const {
				return m_numNewBlockElements;
			}

			size_t numNewTransactionElements() const {
				return m_numNewTransactionElements;
			}

		private:
			crypto::KeyPair m_signerKeyPair;
			test::LocalNodeTestState m_state;
			std::shared_ptr<plugins::PluginManager> m_pPluginManager;
			std::unique_ptr<cache::MemoryUtCache> m_pUnconfirmedTransactionsCache;
			std::atomic<size_t> m_numNewBlockElements;
			std::atomic<size_t> m_numNewTransactionElements;

			ServiceType m_service;
			thread::MultiServicePool m_pool;
		};
	}

	// region boot / shutdown

	TEST(DispatcherServiceTests, CanBootService) {
		// Assert:
		test::AssertCanBootDispatcherService<TestContext>(6, 4);
	}

	TEST(DispatcherServiceTests, CanShutdownService) {
		// Assert:
		test::AssertCanShutdownDispatcherService<TestContext>();
	}

	// endregion

	// region consume

	TEST(DispatcherServiceTests, CanConsumeBlockRange) {
		// Assert:
		test::AssertCanConsumeBlockRange<TestContext>(test::CreateBlockEntityRange(1), [](const auto& context) {
			// - the block was not forwarded to the sink
			EXPECT_EQ(0u, context.numNewBlockElements());
			EXPECT_EQ(0u, context.numNewTransactionElements());
		});
	}

	TEST(DispatcherServiceTests, CanConsumeBlockRangeCompletionAware_InvalidElement) {
		// Assert:
		test::AssertCanConsumeBlockRangeCompletionAware<TestContext>(test::CreateBlockEntityRange(1), [](const auto& context) {
			// - the block was not forwarded to the sink
			EXPECT_EQ(0u, context.numNewBlockElements());
			EXPECT_EQ(0u, context.numNewTransactionElements());
		});
	}

	TEST(DispatcherServiceTests, CanConsumeBlockRangeCompletionAware_ValidElement) {
		// Arrange:
		auto pNextBlock = test::CreateValidBlockForDispatcherTests(GetNemesisAccountKeyPair());
		auto range = test::CreateEntityRange({ pNextBlock.get() });

		// Assert:
		test::AssertCanConsumeBlockRangeCompletionAware<TestContext>(std::move(range), [](const auto& context) {
			// - the block was forwarded to the sink
			EXPECT_EQ(1u, context.numNewBlockElements());
			EXPECT_EQ(0u, context.numNewTransactionElements());
		});
	}

	TEST(DispatcherServiceTests, CanConsumeTransactionRange_InvalidElement) {
		// Assert:
		test::AssertCanConsumeTransactionRange<TestContext>(test::CreateTransactionEntityRange(1), [](const auto& context) {
			// - the transaction was not forwarded to the sink
			EXPECT_EQ(0u, context.numNewBlockElements());
			EXPECT_EQ(0u, context.numNewTransactionElements());
		});
	}

	TEST(DispatcherServiceTests, CanConsumeTransactionRange_ValidElement) {
		// Arrange:
		auto pValidTransaction = test::GenerateRandomTransaction();
		auto range = test::CreateEntityRange({ pValidTransaction.get() });

		// Assert:
		test::AssertCanConsumeTransactionRange<TestContext>(std::move(range), [](const auto& context) {
			WAIT_FOR_ONE_EXPR(context.numNewTransactionElements());

			// - the transaction was forwarded to the sink
			EXPECT_EQ(0u, context.numNewBlockElements());
			EXPECT_EQ(1u, context.numNewTransactionElements());
		});
	}

	// endregion
}}}
