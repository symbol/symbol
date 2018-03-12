#include "sync/src/NetworkPacketWritersService.h"
#include "catapult/api/ChainPackets.h"
#include "catapult/net/VerifyPeer.h"
#include "tests/test/local/PacketWritersServiceTestUtils.h"
#include "tests/test/local/ServiceTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace sync {

#define TEST_CLASS NetworkPacketWritersServiceTests

	namespace {
		struct NetworkPacketWritersServiceTraits {
			static constexpr auto Counter_Name = "WRITERS";
			static constexpr auto Num_Expected_Services = 1;

			static constexpr auto GetWriters = GetPacketWriters;
			static constexpr auto CreateRegistrar = CreateNetworkPacketWritersServiceRegistrar;
		};

		using TestContext = test::ServiceLocatorTestContext<NetworkPacketWritersServiceTraits>;

		struct Mixin {
			using TraitsType = NetworkPacketWritersServiceTraits;
			using TestContextType = TestContext;
		};
	}

	ADD_SERVICE_REGISTRAR_INFO_TEST(NetworkPacketWriters, Initial)

	DEFINE_PACKET_WRITERS_SERVICE_TESTS(TEST_CLASS, Mixin)

	// region packetIoPickers

	TEST(TEST_CLASS, WritersAreRegisteredInPacketIoPickers) {
		// Arrange: create a (tcp) server
		auto pPool = test::CreateStartedIoServiceThreadPool();
		auto serverKeyPair = test::GenerateKeyPair();
		test::SpawnPacketServerWork(pPool->service(), [&service = pPool->service(), &serverKeyPair](const auto& pServer) {
			net::VerifyClient(pServer, serverKeyPair, [&service](auto, const auto&) {});
		});

		// Act: create and boot the service
		TestContext context;
		context.boot();
		auto pickers = context.testState().state().packetIoPickers();

		// - get the packet writers and attempt to connect to the server
		test::ConnectToLocalHost(*GetPacketWriters(context.locator()), serverKeyPair.publicKey());

		// Assert: the writers are registered with role `Peer`
		EXPECT_EQ(1u, pickers.pickMatching(utils::TimeSpan::FromSeconds(1), ionet::NodeRoles::Peer).size());
		EXPECT_EQ(0u, pickers.pickMatching(utils::TimeSpan::FromSeconds(1), ionet::NodeRoles::Api).size());
	}

	// endregion

	// region packetPayloadSink

	TEST(TEST_CLASS, PacketPayloadsAreBroadcastViaWriters) {
		// Arrange: create a (tcp) server
		ionet::ByteBuffer packetBuffer;
		auto pPool = test::CreateStartedIoServiceThreadPool();
		auto serverKeyPair = test::GenerateKeyPair();
		test::SpawnPacketServerWork(pPool->service(), [&service = pPool->service(), &packetBuffer, &serverKeyPair](const auto& pServer) {
			// - verify the client
			net::VerifyClient(pServer, serverKeyPair, [&service, &packetBuffer, pServer](auto, const auto&) {
				// - read the packet and copy it into packetBuffer
				test::AsyncReadIntoBuffer(service, *pServer, packetBuffer);
			});
		});

		// - create and boot the service
		TestContext context;
		context.boot();
		auto sink = context.testState().state().hooks().packetPayloadSink();

		// - get the packet writers and attempt to connect to the server
		test::ConnectToLocalHost(*GetPacketWriters(context.locator()), serverKeyPair.publicKey());

		// Sanity: a single connection was accepted
		EXPECT_EQ(1u, context.counter(NetworkPacketWritersServiceTraits::Counter_Name));

		// Act: broadcast a (transaction) payload to the server
		auto pTransaction = std::shared_ptr<const model::Transaction>(test::GenerateRandomTransaction());
		sink(ionet::PacketPayload::FromEntity(ionet::PacketType::Undefined, pTransaction));

		// - wait for the test to complete
		pPool->join();

		// Assert: the server received the broadcasted entity
		ASSERT_FALSE(packetBuffer.empty());
		EXPECT_EQ(*pTransaction, test::CoercePacketToEntity<model::Transaction>(packetBuffer));
	}

	// endregion

	// region remoteChainHeightsRetriever

	namespace {
		using PacketSockets = std::vector<std::shared_ptr<ionet::PacketSocket>>;

		void EstablishConnections(
				size_t numConnections,
				boost::asio::io_service& service,
				const TestContext& context,
				PacketSockets& serverSockets) {
			// Arrange:
			auto pWriters = GetPacketWriters(context.locator());

			// - connect to the desired number of peers
			test::TcpAcceptor acceptor(service);
			for (auto i = 0u; i < numConnections; ++i) {
				// - connect to nodes with different identities
				auto peerKeyPair = test::GenerateKeyPair();
				ionet::Node node(peerKeyPair.publicKey(), test::CreateLocalHostNodeEndpoint(), ionet::NodeMetadata());

				std::atomic<size_t> numCallbacks(0);
				test::SpawnPacketServerWork(acceptor, [&](const auto& pSocket) {
					serverSockets.push_back(pSocket);
					net::VerifyClient(pSocket, peerKeyPair, [&](auto, const auto&) {
						++numCallbacks;
					});
				});

				pWriters->connect(node, [&](auto) {
					++numCallbacks;
				});

				// - wait for both verifications to complete
				WAIT_FOR_VALUE(2u, numCallbacks);
			}

			// - wait for all connections
			EXPECT_EQ(numConnections, context.counter(NetworkPacketWritersServiceTraits::Counter_Name));
		}

		void AssertReturnedHeights(size_t numHeightPeers, size_t numConnections, const std::vector<Height>& expectedHeights) {
			// Arrange:
			TestContext context;
			context.boot();

			auto retriever = context.testState().state().hooks().remoteChainHeightsRetriever();

			// - connect to the desired number of peers
			auto pPool = test::CreateStartedIoServiceThreadPool();
			PacketSockets serverSockets;
			EstablishConnections(numConnections, pPool->service(), context, serverSockets);
			CATAPULT_LOG(debug) << "established " << numConnections << " connection(s)";

			// - write the heights to the server sockets
			auto i = 1u;
			for (const auto& pSocket : serverSockets) {
				auto pPacket = ionet::CreateSharedPacket<api::ChainInfoResponse>();
				pPacket->Height = Height(i + (0 == i % 2 ? 100u : 0u));
				pSocket->write(pPacket, [](auto) {});
				++i;
			}

			// Act:
			auto heights = retriever(numHeightPeers).get();

			// Assert:
			EXPECT_EQ(expectedHeights, heights) << "numHeightPeers " << numHeightPeers << ", numConnections " << numConnections;
		}
	}

	TEST(TEST_CLASS, RemoteChainHeightsRetrieverReturnsEmptyHeightVectorIfNoPacketIosAreAvailable) {
		// Assert:
		AssertReturnedHeights(3, 0, {});
	}

	TEST(TEST_CLASS, RemoteChainHeightsRetrieverReturnsLessThanNumPeersHeightsIfNotEnoughPacketIosAreAvailable) {
		// Assert:
		AssertReturnedHeights(3, 1, { { Height(1) } });
		AssertReturnedHeights(3, 2, { { Height(1), Height(102) } });
	}

	TEST(TEST_CLASS, RemoteChainHeightsRetrieverReturnsNumPeersHeightsIfEnoughPacketIosAreAvailable) {
		// Assert:
		AssertReturnedHeights(3, 3, { { Height(1), Height(102), Height(3) } });
		AssertReturnedHeights(3, 10, { { Height(1), Height(102), Height(3) } });
	}

	// endregion
}}
