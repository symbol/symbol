#include "catapult/local/api/DispatcherService.h"
#include "catapult/cache/MemoryUtCache.h"
#include "catapult/model/ChainScore.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/test/core/mocks/MockTransaction.h"
#include "tests/test/local/DispatcherServiceTestUtils.h"
#include "tests/test/local/EntityFactory.h"
#include "tests/test/local/LocalNodeTestState.h"
#include "tests/test/local/LocalTestUtils.h"
#include "tests/test/nodeps/MijinConstants.h"
#include "tests/TestHarness.h"

namespace catapult { namespace local { namespace api {

	namespace {
		using ServiceType = DispatcherService;

		crypto::KeyPair GetNemesisAccountKeyPair() {
			return crypto::KeyPair::FromString(test::Mijin_Test_Private_Keys[0]); // use a nemesis account
		}

		class TestContext {
		public:
			TestContext()
					: m_signerKeyPair(GetNemesisAccountKeyPair()) // use a nemesis account
					, m_state(test::CreateCatapultCacheForDispatcherTests<test::CoreSystemCacheFactory>())
					, m_pPluginManager(test::CreateDefaultPluginManager())
					, m_pUnconfirmedTransactionsCache(test::CreateUnconfirmedTransactionsCache())
					, m_apiState(m_state.ref())
					, m_numNewTransactionElements(0)
					, m_service(
							m_apiState,
							*m_pPluginManager,
							test::CreateViewProvider(*m_pUnconfirmedTransactionsCache),
							*m_pUnconfirmedTransactionsCache,
							[this](auto&&) {
								CATAPULT_LOG(debug) << "service forwarded new transaction element";
								++m_numNewTransactionElements;
							})
					, m_pool(2, "dispatcher") {
				test::InitializeCatapultCacheForDispatcherTests(m_state.ref().Cache, m_signerKeyPair);
				m_apiState.subscribeStateChange([&contextChainScore = m_chainScore](const auto& chainScore, const auto&) {
					contextChainScore = chainScore;
				});

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
			size_t numNewTransactionElements() const {
				return m_numNewTransactionElements;
			}

			const model::ChainScore& score() const {
				return m_chainScore;
			}

		private:
			crypto::KeyPair m_signerKeyPair;
			test::LocalNodeTestState m_state;
			std::shared_ptr<plugins::PluginManager> m_pPluginManager;
			std::unique_ptr<cache::MemoryUtCache> m_pUnconfirmedTransactionsCache;
			LocalNodeApiState m_apiState;
			model::ChainScore m_chainScore;
			std::atomic<size_t> m_numNewTransactionElements;

			ServiceType m_service;
			thread::MultiServicePool m_pool;
		};
	}

	// region boot / shutdown

	TEST(DispatcherServiceTests, CanBootService) {
		// Assert:
		test::AssertCanBootDispatcherService<TestContext>(3, 3);
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
			EXPECT_EQ(0u, context.numNewTransactionElements());
		});
	}

	TEST(DispatcherServiceTests, CanConsumeBlockRangeCompletionAware_InvalidElement) {
		// Assert:
		test::AssertCanConsumeBlockRangeCompletionAware<TestContext>(test::CreateBlockEntityRange(1), [](const auto& context) {
			// - the block was not forwarded to the sink
			EXPECT_EQ(0u, context.numNewTransactionElements());

			// - score listener shouldn't have been called for an invalid element, so chain score should be unchanged (zero)
			EXPECT_EQ(model::ChainScore(), context.score());
		});
	}

	TEST(DispatcherServiceTests, CanConsumeBlockRangeCompletionAware_ValidElement) {
		// Arrange:
		auto pNextBlock = test::CreateValidBlockForDispatcherTests(GetNemesisAccountKeyPair());
		auto range = test::CreateEntityRange({ pNextBlock.get() });

		// Assert:
		test::AssertCanConsumeBlockRangeCompletionAware<TestContext>(std::move(range), [](const auto& context) {
			// - the block was not forwarded to the sink
			EXPECT_EQ(0u, context.numNewTransactionElements());

			// - score listener should have been called, so chain score should be changed (non-zero)
			EXPECT_EQ(model::ChainScore(99999999999940u), context.score());
		});
	}

	TEST(DispatcherServiceTests, CanConsumeTransactionRange_InvalidElement) {
		// Arrange: create and boot the service
		TestContext context;
		auto& service = context.service();
		auto factory = service.createTransactionRangeConsumerFactory()(disruptor::InputSource::Local);
		context.boot();

		// Act: api transaction dispatcher does not run any validation, so simulate an invalid element by sending two copies of
		//      the same range (the HashCheckConsumer should reject the second)
		auto range = test::CreateTransactionEntityRange(1);
		factory(model::TransactionRange::CopyRange(range));
		WAIT_FOR_VALUE_EXPR(service.numAddedTransactionElements(), 1u);
		WAIT_FOR_VALUE_EXPR(context.numNewTransactionElements(), 1u);

		factory(model::TransactionRange::CopyRange(range));
		WAIT_FOR_VALUE_EXPR(service.numAddedTransactionElements(), 2u);

		// - wait a bit to give the consumer time to consume more if there is a bug in the implementation
		test::Pause();

		// Assert:
		EXPECT_EQ(0u, service.numAddedBlockElements());
		EXPECT_EQ(2u, service.numAddedTransactionElements());
		EXPECT_EQ(1u, context.numNewTransactionElements());
	}

	TEST(DispatcherServiceTests, CanConsumeTransactionRange_ValidElement) {
		// Arrange:
		auto pValidTransaction = test::GenerateRandomTransaction();
		auto range = test::CreateEntityRange({ pValidTransaction.get() });

		// Assert:
		test::AssertCanConsumeTransactionRange<TestContext>(std::move(range), [](const auto& context) {
			WAIT_FOR_ONE_EXPR(context.numNewTransactionElements());

			// - the transaction was forwarded to the sink
			EXPECT_EQ(1u, context.numNewTransactionElements());
		});
	}

	// endregion
}}}
