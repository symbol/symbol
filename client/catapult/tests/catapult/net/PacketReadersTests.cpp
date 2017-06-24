#include "catapult/net/PacketReaders.h"
#include "catapult/crypto/KeyPair.h"
#include "catapult/ionet/PacketSocket.h"
#include "catapult/net/AsyncTcpServer.h"
#include "catapult/net/VerifyPeer.h"
#include "catapult/thread/IoServiceThreadPool.h"
#include "tests/test/core/AddressTestUtils.h"
#include "tests/test/core/ThreadPoolTestUtils.h"
#include "tests/test/net/NodeTestUtils.h"
#include "tests/test/net/SocketTestUtils.h"
#include "tests/test/net/mocks/MockAsyncTcpServerAcceptContext.h"

using catapult::mocks::MockAsyncTcpServerAcceptContext;

namespace catapult { namespace net {

	namespace {
		auto CreateDefaultPacketReaders() {
			return CreatePacketReaders(
					test::CreateStartedIoServiceThreadPool(),
					ionet::ServerPacketHandlers(),
					test::GenerateKeyPair(),
					ConnectionSettings());
		}
	}

	TEST(PacketReadersTests, InitiallyNoConnectionsAreActive) {
		// Act:
		auto pReaders = CreateDefaultPacketReaders();

		// Assert:
		EXPECT_EQ(0u, pReaders->numActiveConnections());
		EXPECT_EQ(0u, pReaders->numActiveReaders());
	}

	TEST(PacketReadersTests, AcceptFailsOnAcceptError) {
		// Arrange:
		auto pReaders = CreateDefaultPacketReaders();

		// Act: on an accept error, the server will pass nullptr
		PeerConnectResult result;
		pReaders->accept(nullptr, [&result](auto acceptResult) { result = acceptResult; });

		// Assert:
		EXPECT_EQ(PeerConnectResult::Socket_Error, result);
		EXPECT_EQ(0u, pReaders->numActiveConnections());
		EXPECT_EQ(0u, pReaders->numActiveReaders());
	}

	namespace {
		struct PacketReadersTestContext {
		public:
			explicit PacketReadersTestContext(const ionet::ServerPacketHandlers& handlers = ionet::ServerPacketHandlers())
					: ServerKeyPair(test::GenerateKeyPair())
					, ClientKeyPair(test::GenerateKeyPair())
					, pPool(test::CreateStartedIoServiceThreadPool())
					, Service(pPool->service())
					, Handlers(handlers)
					, pReaders(CreatePacketReaders(pPool, Handlers, ServerKeyPair, ConnectionSettings()))
			{}

			~PacketReadersTestContext() {
				pReaders->shutdown();
				test::WaitForUnique(pReaders, "pReaders");
				pPool->join();
			}

		public:
			crypto::KeyPair ServerKeyPair;
			crypto::KeyPair ClientKeyPair;
			std::shared_ptr<thread::IoServiceThreadPool> pPool;
			boost::asio::io_service& Service;
			ionet::ServerPacketHandlers Handlers;
			std::shared_ptr<PacketReaders> pReaders;
		};
	}

	TEST(PacketReadersTests, AcceptFailsOnVerifyError) {
		// Arrange:
		PacketReadersTestContext context;
		std::atomic<size_t> numCallbacks(0);

		// Act: start a server and client verify operation
		PeerConnectResult result;
		test::SpawnPacketServerWork(context.Service, [&](const auto& pSocket) -> void {
			auto pAcceptContext = std::make_shared<MockAsyncTcpServerAcceptContext>(context.Service, pSocket);
			context.pReaders->accept(pAcceptContext, [&](auto acceptResult) {
				result = acceptResult;
				++numCallbacks;
			});
		});

		test::SpawnPacketClientWork(context.Service, [&](const auto& pSocket) -> void {
			// - trigger a verify error by closing the socket without responding
			pSocket->close();
			++numCallbacks;
		});

		// - wait for both callbacks to complete and the connection to close
		WAIT_FOR_VALUE(numCallbacks, 2u);
		WAIT_FOR_VALUE_EXPR(context.pReaders->numActiveConnections(), 0u);

		// Assert: the verification should have failed and all connections should have been destroyed
		EXPECT_EQ(PeerConnectResult::Verify_Error, result);
		EXPECT_EQ(0u, context.pReaders->numActiveConnections());
		EXPECT_EQ(0u, context.pReaders->numActiveReaders());
	}

	namespace {
		struct MultiConnectionState {
			std::vector<PeerConnectResult> Results;
			std::vector<std::shared_ptr<ionet::PacketSocket>> ServerSockets;
			std::vector<std::shared_ptr<ionet::PacketSocket>> ClientSockets;
		};

		MultiConnectionState SetupMultiConnectionTest(
				const PacketReadersTestContext& context,
				size_t numConnections) {
			// Act: start multiple server and client verify operations
			MultiConnectionState state;
			for (auto i = 0u; i < numConnections; ++i) {
				std::atomic<size_t> numCallbacks(0);
				test::SpawnPacketServerWork(context.Service, [&](const auto& pSocket) -> void {
					state.ServerSockets.push_back(pSocket);
					auto pAcceptContext = std::make_shared<MockAsyncTcpServerAcceptContext>(context.Service, pSocket);
					context.pReaders->accept(pAcceptContext, [&](auto result) {
						state.Results.push_back(result);
						++numCallbacks;
					});
				});

				bool isServerVerified = false;
				test::SpawnPacketClientWork(context.Service, [&](const auto& pSocket) -> void {
					state.ClientSockets.push_back(pSocket);
					VerifyServer(pSocket, context.ServerKeyPair.publicKey(), context.ClientKeyPair, [&](auto result, const auto&) {
						isServerVerified = VerifyResult::Success == result;
						++numCallbacks;
					});
				});

				// - wait for both verifications to complete and make sure the client verified too
				WAIT_FOR_VALUE(numCallbacks, 2u);
				EXPECT_TRUE(isServerVerified);
			}

			return state;
		}

		using ResultServerClientHandler =
				std::function<void (PeerConnectResult, ionet::PacketSocket&, ionet::PacketSocket&)>;

		void RunConnectedSocketTest(const PacketReadersTestContext& context, const ResultServerClientHandler& handler) {
			// Act: establish a single connection
			auto state = SetupMultiConnectionTest(context, 1);

			// Assert: call the handler
			handler(state.Results.back(), *state.ServerSockets.back(), *state.ClientSockets.back());
		}
	}

	TEST(PacketReadersTests, AcceptSucceedsOnVerifySuccess) {
		// Act:
		PacketReadersTestContext context;
		RunConnectedSocketTest(context, [&](auto result, const auto&, const auto&) {
			// Assert: the verification should have succeeded and the connection should be active
			EXPECT_EQ(PeerConnectResult::Accepted, result);
			EXPECT_EQ(1u, context.pReaders->numActiveConnections());
			EXPECT_EQ(1u, context.pReaders->numActiveReaders());
		});
	}

	TEST(PacketReadersTests, ShutdownClosesAcceptedSocket) {
		// Act:
		PacketReadersTestContext context;
		RunConnectedSocketTest(context, [&](auto, auto& serverSocket, const auto&) {
			// Act: shutdown the readers
			context.pReaders->shutdown();

			// Assert: the server socket was closed
			EXPECT_FALSE(test::IsSocketOpen(serverSocket));
			EXPECT_EQ(0u, context.pReaders->numActiveConnections());
			EXPECT_EQ(0u, context.pReaders->numActiveReaders());
		});
	}

	TEST(PacketReadersTests, CanManageMultipleConnections) {
		// Act: establish multiple connections
		static const auto Num_Connections = 5u;
		PacketReadersTestContext context;
		auto state = SetupMultiConnectionTest(context, Num_Connections);

		// Assert: all connections are active
		EXPECT_EQ(Num_Connections, state.Results.size());
		for (auto result : state.Results)
			EXPECT_EQ(PeerConnectResult::Accepted, result);

		EXPECT_EQ(Num_Connections, context.pReaders->numActiveConnections());
		EXPECT_EQ(Num_Connections, context.pReaders->numActiveReaders());
	}

	namespace {
		void RunConnectingSocketTest(const PacketReadersTestContext& context, const ResultServerClientHandler& handler) {
			std::atomic<size_t> numCallbacks(0);

			// Act: start a server verify operation that the client does not respond to
			//      (use a result shared_ptr so that the accept callback is valid even after this function returns)
			auto pResult = std::make_shared<PeerConnectResult>(static_cast<PeerConnectResult>(-1));
			std::shared_ptr<ionet::PacketSocket> pServerSocket;
			test::SpawnPacketServerWork(context.Service, [&, pResult](const auto& pSocket) -> void {
				pServerSocket = pSocket;
				auto pAcceptContext = std::make_shared<MockAsyncTcpServerAcceptContext>(context.Service, pSocket);
				context.pReaders->accept(pAcceptContext, [&, pResult](auto acceptResult) {
					// note that this is not expected to get called until shutdown because the client doesn't read
					// or write any data
					*pResult = acceptResult;
				});
				++numCallbacks;
			});

			std::shared_ptr<ionet::PacketSocket> pClientSocket;
			test::SpawnPacketClientWork(context.Service, [&](const auto& pSocket) {
				pClientSocket = pSocket;
				++numCallbacks;
			});

			// - wait for the initial work to complete
			WAIT_FOR_VALUE(numCallbacks, 2u);

			// Assert: the server accept callback was neved called
			EXPECT_EQ(static_cast<PeerConnectResult>(-1), *pResult);

			// - call the test handler
			handler(*pResult, *pServerSocket, *pClientSocket);
		}
	}

	TEST(PacketReadersTests, VerifyingConnectionIsIncludedInNumActiveConnections) {
		// Act:
		PacketReadersTestContext context;
		RunConnectingSocketTest(context, [&](auto, const auto&, const auto&) {
			// Assert: the verifying connection is active
			EXPECT_EQ(1u, context.pReaders->numActiveConnections());
			EXPECT_EQ(0u, context.pReaders->numActiveReaders());
		});
	}

	TEST(PacketReadersTests, ShutdownClosesVerifyingSocket) {
		// Act:
		PacketReadersTestContext context;
		RunConnectingSocketTest(context, [&](auto, auto& serverSocket, const auto&) {
			// Act: shutdown the readers
			context.pReaders->shutdown();

			// Assert: the server socket was closed
			EXPECT_FALSE(test::IsSocketOpen(serverSocket));
			EXPECT_EQ(0u, context.pReaders->numActiveConnections());
			EXPECT_EQ(0u, context.pReaders->numActiveReaders());
		});
	}

	namespace {
		void RunPacketHandlerTest(size_t numConnections) {
			constexpr size_t Tag_Index = sizeof(ionet::Packet);

			// Arrange: set up a handler that copies packets
			ionet::ServerPacketHandlers handlers;
			std::atomic<size_t> numPacketsRead(0);
			std::vector<ionet::ByteBuffer> receiveBuffers(numConnections);
			test::RegisterDefaultHandler(handlers, [&](const auto& packet, const auto&) {
				auto receiveBuffer = test::CopyPacketToBuffer(packet);
				size_t tag = receiveBuffer[Tag_Index];

				CATAPULT_LOG(debug) << "handling packet " << tag;
				receiveBuffers[tag] = receiveBuffer;
				++numPacketsRead;
			});

			// - connect to the specified number of nodes
			PacketReadersTestContext context(handlers);
			auto state = SetupMultiConnectionTest(context, numConnections);

			// Act: send a single (different) packet to each socket and uniquely tag each packet
			std::vector<ionet::ByteBuffer> sendBuffers;
			for (const auto& pSocket : state.ClientSockets) {
				auto sendBuffer = test::GenerateRandomPacketBuffer(62);
				size_t tag = sendBuffers.size();
				sendBuffer[Tag_Index] = static_cast<uint8_t>(tag);

				CATAPULT_LOG(debug) << "writing packet " << tag;
				pSocket->write(test::BufferToPacket(sendBuffer), [](auto) {});
				sendBuffers.push_back(sendBuffer);
			}

			// - wait for all packets to be read
			WAIT_FOR_VALUE(numPacketsRead, numConnections);

			// Assert: the handler was called once for each socket with the corresponding sent packet
			for (auto i = 0u; i < numConnections; ++i) {
				auto sendBufferHex = test::ToHexString(sendBuffers[i]);
				auto receiveBufferHex = test::ToHexString(receiveBuffers[i]);
				EXPECT_EQ(sendBufferHex, receiveBufferHex) << "tagged packet " << i;
			}
		}
	}

	TEST(PacketReadersTests, SingleReaderPassesPayloadToHandlers) {
		// Assert:
		RunPacketHandlerTest(1);
	}

	TEST(PacketReadersTests, MultipleReadersPassPayloadToHandlers) {
		// Assert:
		RunPacketHandlerTest(4);
	}
}}
