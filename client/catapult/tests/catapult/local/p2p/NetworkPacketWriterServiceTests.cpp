#include "catapult/local/p2p/NetworkPacketWriterService.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/test/local/LocalTestUtils.h"
#include "tests/test/local/NetworkServiceTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace local { namespace p2p {

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

		public:
			const auto& publicKey() {
				return m_keyPair.publicKey();
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
		test::AssertCanBootWriters<TestContext>([](const auto& service) {
			EXPECT_EQ(0u, service.numActiveBroadcastWriters());
		});
	}

	TEST(NetworkPacketWriterServiceTests, CanShutdownService) {
		// Assert:
		test::AssertCanShutdownWriters<TestContext>([](const auto& service) {
			EXPECT_EQ(Sentinel_Stats_Value, service.numActiveBroadcastWriters());
		});
	}

	// endregion

	// region broadcast tests

	namespace {
		template<typename TTraits>
		void AssertCanBroadcastBlock() {
			// Arrange:
			auto pBlock = std::shared_ptr<model::Block>(test::GenerateDeterministicBlock());

			// Assert:
			TTraits::AssertCanBroadcastEntity(
					[&pBlock](auto& service) { service.createNewBlockSink()(pBlock); },
					[&pBlock](const auto& buffer) {
						// Assert: the server received the broadcasted entity
						EXPECT_EQ(*pBlock, test::CoercePacketToEntity<model::Block>(buffer));
					});
		}
	}

#define ADD_BROADCAST_TESTS(TAG) \
	TEST(NetworkPacketWriterServiceTests, CanBroadcastBlock_##TAG) { AssertCanBroadcastBlock<TAG##Traits>(); } \
	TEST(NetworkPacketWriterServiceTests, CanBroadcastTransaction_##TAG) { test::AssertCanBroadcastTransaction<TAG##Traits>(); }

	// endregion

	// region connect and broadcast to writers

	namespace {
		void AssertNoBroadcastWriters(const ServiceType& service) {
			// Assert: no broadcast writers are connected
			EXPECT_EQ(0u, service.numActiveBroadcastWriters());
		}
	}

	TEST(NetworkPacketWriterServiceTests, CanConnectToExternalServerUsingPacketWriters) {
		// Assert:
		test::AssertWritersCanAcceptConnection<TestContext>(AssertNoBroadcastWriters);
	}

	namespace {
		struct Connector {
			static void ConnectToExternalWriter(ServiceType& service, const Key& serverPublicKey) {
				test::ConnectToExternalWriter(service, serverPublicKey, AssertNoBroadcastWriters);
			}
		};

		using WritersTraits = test::BasicWritersTraits<TestContext, Connector>;
	}

	ADD_BROADCAST_TESTS(Writers)

	// endregion

	// region broadcast to broadcast writers

	namespace {
		constexpr unsigned short Local_Host_Api_Port = test::Local_Host_Port + 1;

		std::shared_ptr<ionet::PacketIo> AcceptBroadcastWriter(
				boost::asio::io_service& ioService,
				ServiceType& service,
				const Key& serverPublicKey) {
			// Act: connect to the server as a broadcast writer
			auto pIo = test::ConnectToLocalHost(ioService, Local_Host_Api_Port, serverPublicKey);

			// Assert: a single connection was accepted
			EXPECT_EQ(0u, service.numActiveWriters());
			EXPECT_EQ(1u, service.numActiveBroadcastWriters());
			return pIo;
		}
	}

	TEST(NetworkPacketWriterServiceTests, CanAcceptConnectionsFromBroadcastWriters) {
		// Arrange: create and boot the service
		TestContext context;
		auto& service = context.service();
		context.boot();

		// Act + Assert: connect to the server as a broadcast writer
		auto pPool = test::CreateStartedIoServiceThreadPool();
		AcceptBroadcastWriter(pPool->service(), service, context.publicKey());
	}

	namespace {
		struct BroadcastWritersTraits {
			template<typename TBroadcastEntity, typename TVerifyReadBuffer>
			static void AssertCanBroadcastEntity(TBroadcastEntity broadcastEntity, TVerifyReadBuffer verifyReadBuffer) {
				// Arrange: create and boot the service
				TestContext context;
				auto& service = context.service();
				context.boot();

				// - connect to the server as a broadcast writer
				auto pPool = test::CreateStartedIoServiceThreadPool();
				auto pIo = AcceptBroadcastWriter(pPool->service(), service, context.publicKey());

				// - set up a read
				ionet::ByteBuffer packetBuffer;
				test::AsyncReadIntoBuffer(*pIo, packetBuffer);

				// Act: broadcast an entity to the server
				broadcastEntity(service);

				// - wait for the test to complete
				pPool->join();

				// Assert: the server received the broadcasted entity
				verifyReadBuffer(packetBuffer);
			}
		};
	}

	ADD_BROADCAST_TESTS(BroadcastWriters)

	// endregion
}}}
