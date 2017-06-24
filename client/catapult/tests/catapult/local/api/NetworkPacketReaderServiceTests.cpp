#include "catapult/local/api/NetworkPacketReaderService.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/test/core/mocks/MockTransaction.h"
#include "tests/test/local/LocalNodeTestState.h"
#include "tests/test/local/NetworkServiceTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace local { namespace api {

	namespace {
		using ServiceType = NetworkPacketReaderService;

		class TestContext {
		public:
			TestContext()
					: m_keyPair(test::GenerateKeyPair())
					, m_pTransactionRegistry(mocks::CreateDefaultTransactionRegistry())
					, m_numConsumedTransactionElements(0)
					, m_service(
							m_keyPair,
							m_state.cref(),
							*m_pTransactionRegistry,
							[this](auto&&) {
								CATAPULT_LOG(debug) << "service consumed transaction element";
								++m_numConsumedTransactionElements;
							})
					, m_pool(2, "reader")
			{}

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
			size_t numConsumedTransactionElements() const {
				return m_numConsumedTransactionElements;
			}

			const auto& publicKey() const {
				return m_keyPair.publicKey();
			}

		private:
			crypto::KeyPair m_keyPair;
			test::LocalNodeTestState m_state;
			std::unique_ptr<model::TransactionRegistry> m_pTransactionRegistry;
			std::atomic<size_t> m_numConsumedTransactionElements;

			ServiceType m_service;
			thread::MultiServicePool m_pool;
		};
	}

	// region boot / shutdown

	TEST(NetworkPacketReaderServiceTests, CanBootService) {
		// Assert:
		test::AssertCanBootReaders<TestContext>([](const auto& context) {
			EXPECT_EQ(0u, context.numConsumedTransactionElements());
		});
	}

	TEST(NetworkPacketReaderServiceTests, CanShutdownService) {
		// Assert:
		test::AssertCanShutdownReaders<TestContext>([](const auto& context) {
			EXPECT_EQ(0u, context.numConsumedTransactionElements());
		});
	}

	// endregion

	// region basic connection

	TEST(NetworkPacketReaderServiceTests, CanAcceptExternalConnection) {
		// Act:
		test::RunSingleReaderTest(TestContext(), [](const auto& context, const auto&, const auto&) {
			// Assert: nothing was consumed
			EXPECT_EQ(0u, context.numConsumedTransactionElements());
		});
	}

	// endregion

	// region push entity handlers

	TEST(NetworkPacketReaderServiceTests, CannotPushBlock) {
		// Arrange:
		test::RunSingleReaderTest(TestContext(), [](const auto& context, const auto& service, auto& io) {
			// Act: push a block
			io.write(test::GenerateRandomBlockPacket(), [](auto) {});
			WAIT_FOR_VALUE_EXPR(service.numActiveReaders(), 0u);

			// Assert: the connection was closed
			EXPECT_EQ(0u, context.numConsumedTransactionElements());
			EXPECT_EQ(0u, service.numActiveReaders());
		});
	}

	TEST(NetworkPacketReaderServiceTests, CanPushTransaction) {
		// Arrange:
		test::RunSingleReaderTest(TestContext(), [](const auto& context, const auto& service, auto& io) {
			// Act: push a transaction
			io.write(test::GenerateRandomTransactionPacket(), [](auto) {});
			WAIT_FOR_ONE_EXPR(context.numConsumedTransactionElements());

			// Assert: a transaction was consumed
			EXPECT_EQ(1u, context.numConsumedTransactionElements());
			EXPECT_EQ(1u, service.numActiveReaders());
		});
	}

	// endregion
}}}
