#include "transactionsink/src/TransactionSinkService.h"
#include "tests/test/core/PacketTestUtils.h"
#include "tests/test/core/mocks/MockTransaction.h"
#include "tests/test/local/ServiceLocatorTestContext.h"
#include "tests/test/local/ServiceTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace transactionsink {

#define TEST_CLASS TransactionSinkServiceTests

	namespace {
		struct TransactionSinkServiceTraits {
			static constexpr auto CreateRegistrar = CreateTransactionSinkServiceRegistrar;
		};

		class TestContext : public test::ServiceLocatorTestContext<TransactionSinkServiceTraits> {
		public:
			explicit TestContext(bool isChainSynced = true) : m_numPushedTransactionElements(0) {
				// set up hooks (only increment the counters if the input source is correct)
				auto& hooks = testState().state().hooks();
				hooks.setChainSyncedPredicate([isChainSynced]() { return isChainSynced; });
				hooks.setTransactionRangeConsumerFactory([&counter = m_numPushedTransactionElements](auto source) {
					return [&counter, source](auto&&) { counter += disruptor::InputSource::Remote_Push == source ? 1 : 0; };
				});

				// the service needs to be able to parse the mock transactions sent to it
				testState().pluginManager().addTransactionSupport(mocks::CreateMockTransactionPlugin());
			}

		public:
			size_t numPushedTransactionElements() {
				return m_numPushedTransactionElements;
			}

		private:
			size_t m_numPushedTransactionElements;
		};
	}

	// region basic

	ADD_SERVICE_REGISTRAR_INFO_TEST(TransactionSink, Post_Range_Consumers)

	TEST(TEST_CLASS, NoServicesOrCountersAreRegistered) {
		// Assert:
		test::AssertNoServicesOrCountersAreRegistered<TestContext>();
	}

	TEST(TEST_CLASS, PacketHandlersAreRegistered) {
		// Assert:
		test::AssertSinglePacketHandlerIsRegistered<TestContext>(ionet::PacketType::Push_Transactions);
	}

	// endregion

	// region hook wiring

	namespace {
		void AssertTransactionPush(bool isChainSynced, size_t numExpectedPushes) {
			// Arrange:
			TestContext context(isChainSynced);
			context.boot();

			// Act:
			ionet::ServerPacketHandlerContext handlerContext({}, "");
			const auto& handlers = context.testState().state().packetHandlers();
			handlers.process(*test::GenerateRandomTransactionPacket(), handlerContext);

			// Assert:
			EXPECT_EQ(numExpectedPushes, context.numPushedTransactionElements());
		}
	}

	TEST(TEST_CLASS, CanPushTransactionWhenSynced) {
		// Assert:
		AssertTransactionPush(true, 1);
	}

	TEST(TEST_CLASS, CannotPushTransactionWhenNotSynced) {
		// Assert:
		AssertTransactionPush(false, 0);
	}

	// endregion
}}
