#include "catapult/local/api/NetworkPacketWriterService.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/test/local/LocalTestUtils.h"
#include "tests/test/local/NetworkServiceTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace local { namespace api {

	namespace {
		using ServiceType = NetworkPacketWriterService;

		class TestContext {
		public:
			TestContext()
					: m_keyPair(test::GenerateKeyPair())
					, m_config(test::CreatePrototypicalLocalNodeConfiguration())
					, m_service(m_keyPair, m_config)
					, m_pool(2, "writer")
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

		private:
			crypto::KeyPair m_keyPair;
			config::LocalNodeConfiguration m_config;

			ServiceType m_service;
			thread::MultiServicePool m_pool;
		};
	}

	// region boot / shutdown

	TEST(NetworkPacketWriterServiceTests, CanBootService) {
		// Assert:
		test::AssertCanBootWriters<TestContext>([](const auto&) {});
	}

	TEST(NetworkPacketWriterServiceTests, CanShutdownService) {
		// Assert:
		test::AssertCanShutdownWriters<TestContext>([](const auto&) {});
	}

	// endregion

	// region broadcast tests

#define ADD_BROADCAST_TESTS(TAG) \
	TEST(NetworkPacketWriterServiceTests, CanBroadcastTransaction_##TAG) { test::AssertCanBroadcastTransaction<TAG##Traits>(); }

	// endregion

	// region connect and broadcast to writers

	TEST(NetworkPacketWriterServiceTests, CanConnectToExternalServerUsingPacketWriters) {
		// Assert:
		test::AssertWritersCanAcceptConnection<TestContext>([](const auto&) {});
	}

	namespace {
		struct Connector {
			static void ConnectToExternalWriter(ServiceType& service, const Key& serverPublicKey) {
				test::ConnectToExternalWriter(service, serverPublicKey, [](const auto&) {});
			}
		};

		using WritersTraits = test::BasicWritersTraits<TestContext, Connector>;
	}

	ADD_BROADCAST_TESTS(Writers)

	// endregion
}}}
