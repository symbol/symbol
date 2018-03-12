#include "catapult/net/ClientConnector.h"
#include "catapult/crypto/KeyPair.h"
#include "catapult/ionet/PacketSocket.h"
#include "catapult/net/VerifyPeer.h"
#include "catapult/thread/IoServiceThreadPool.h"
#include "tests/test/core/AddressTestUtils.h"
#include "tests/test/core/ThreadPoolTestUtils.h"
#include "tests/test/net/SocketTestUtils.h"

namespace catapult { namespace net {

#define TEST_CLASS ClientConnectorTests

	namespace {
		auto CreateDefaultClientConnector() {
			return CreateClientConnector(test::CreateStartedIoServiceThreadPool(), test::GenerateKeyPair(), ConnectionSettings());
		}

		struct ConnectorTestContext {
		public:
			explicit ConnectorTestContext(const ConnectionSettings& settings = ConnectionSettings())
					: ServerKeyPair(test::GenerateKeyPair())
					, ClientKeyPair(test::GenerateKeyPair())
					, pPool(test::CreateStartedIoServiceThreadPool())
					, Service(pPool->service())
					, pConnector(CreateClientConnector(pPool, ServerKeyPair, settings))
			{}

			~ConnectorTestContext() {
				pConnector->shutdown();
				test::WaitForUnique(pConnector, "pConnector");
				pPool->join();
			}

		public:
			crypto::KeyPair ServerKeyPair;
			crypto::KeyPair ClientKeyPair;
			std::shared_ptr<thread::IoServiceThreadPool> pPool;
			boost::asio::io_service& Service;
			std::shared_ptr<ClientConnector> pConnector;
		};
	}

	TEST(TEST_CLASS, InitiallyNoConnectionsAreActive) {
		// Act:
		auto pConnector = CreateDefaultClientConnector();

		// Assert:
		EXPECT_EQ(0u, pConnector->numActiveConnections());
	}

	TEST(TEST_CLASS, AcceptFailsOnAcceptError) {
		// Arrange:
		auto pConnector = CreateDefaultClientConnector();

		// Act: on an accept error, the server will pass nullptr
		PeerConnectResult result;
		Key clientKey;
		pConnector->accept(nullptr, [&](auto acceptResult, const auto& key) {
			result = acceptResult;
			clientKey = key;
		});

		// Assert:
		EXPECT_EQ(PeerConnectResult::Socket_Error, result);
		EXPECT_EQ(Key(), clientKey);
		EXPECT_EQ(0u, pConnector->numActiveConnections());
	}

	TEST(TEST_CLASS, AcceptFailsOnVerifyError) {
		// Arrange:
		ConnectorTestContext context;
		std::atomic<size_t> numCallbacks(0);

		// Act: start a server and client verify operation
		PeerConnectResult result;
		Key clientKey;
		test::SpawnPacketServerWork(context.Service, [&](const auto& pSocket) {
			context.pConnector->accept(pSocket, [&](auto acceptResult, const auto& key) {
				result = acceptResult;
				clientKey = key;
				++numCallbacks;
			});
		});

		test::SpawnPacketClientWork(context.Service, [&](const auto& pSocket) {
			// - trigger a verify error by closing the socket without responding
			pSocket->close();
			++numCallbacks;
		});

		// - wait for both callbacks to complete and the connection to close
		WAIT_FOR_VALUE(2u, numCallbacks);
		WAIT_FOR_ZERO_EXPR(context.pConnector->numActiveConnections());

		// Assert: the verification should have failed and all connections should have been destroyed
		EXPECT_EQ(PeerConnectResult::Verify_Error, result);
		EXPECT_EQ(Key(), clientKey);
		EXPECT_EQ(0u, context.pConnector->numActiveConnections());
	}

	namespace {
		struct MultiConnectionState {
			std::vector<PeerConnectResult> Results;
			std::vector<Key> ClientKeys;
			std::vector<std::shared_ptr<ionet::PacketSocket>> ServerSockets;
			std::vector<std::shared_ptr<ionet::PacketSocket>> ClientSockets;
		};

		MultiConnectionState SetupMultiConnectionTest(const ConnectorTestContext& context, size_t numConnections) {
			// Act: start multiple server and client verify operations
			MultiConnectionState state;
			test::TcpAcceptor acceptor(context.Service);
			for (auto i = 0u; i < numConnections; ++i) {
				std::atomic<size_t> numCallbacks(0);
				test::SpawnPacketServerWork(acceptor, [&](const auto& pSocket) {
					state.ServerSockets.push_back(pSocket);
					context.pConnector->accept(pSocket, [&](auto result, const auto& key) {
						state.Results.push_back(result);
						state.ClientKeys.push_back(key);
						++numCallbacks;
					});
				});

				test::SpawnPacketClientWork(context.Service, [&](const auto& pSocket) {
					state.ClientSockets.push_back(pSocket);
					VerifyServer(pSocket, context.ServerKeyPair.publicKey(), context.ClientKeyPair, [&](auto, const auto&) {
						++numCallbacks;
					});
				});

				// - wait for both verifications to complete
				WAIT_FOR_VALUE(2u, numCallbacks);
			}

			return state;
		}

		using ConnectedSocketHandler = consumer<
				PeerConnectResult,
				const Key&,
				std::shared_ptr<ionet::PacketSocket>&,
				std::shared_ptr<ionet::PacketSocket>&>;

		void RunConnectedSocketTest(const ConnectorTestContext& context, const ConnectedSocketHandler& handler) {
			// Act: establish a single connection
			auto state = SetupMultiConnectionTest(context, 1);

			// Assert: call the handler
			handler(state.Results.back(), state.ClientKeys.back(), state.ServerSockets.back(), state.ClientSockets.back());
		}
	}

	TEST(TEST_CLASS, AcceptSucceedsOnVerifySuccess) {
		// Act:
		ConnectorTestContext context;
		RunConnectedSocketTest(context, [&](auto result, const auto& clientKey, const auto&, const auto&) {
			// Assert: the verification should have succeeded and the connection should be active
			EXPECT_EQ(PeerConnectResult::Accepted, result);
			EXPECT_EQ(context.ClientKeyPair.publicKey(), clientKey);
			EXPECT_EQ(1u, context.pConnector->numActiveConnections());
		});
	}

	TEST(TEST_CLASS, ShutdownClosesAcceptedSocket) {
		// Act:
		ConnectorTestContext context;
		RunConnectedSocketTest(context, [&](auto, const auto&, const auto& pServerSocket, const auto&) {
			// Act: shutdown the connector
			context.pConnector->shutdown();

			// Assert: the server socket was closed
			EXPECT_FALSE(test::IsSocketOpen(*pServerSocket));
			EXPECT_EQ(0u, context.pConnector->numActiveConnections());
		});
	}

	TEST(TEST_CLASS, CanManageMultipleConnections) {
		// Act: establish multiple connections
		static const auto Num_Connections = 5u;
		ConnectorTestContext context;
		auto state = SetupMultiConnectionTest(context, Num_Connections);

		// Assert: all connections are active
		ASSERT_EQ(Num_Connections, state.Results.size());
		for (auto i = 0u; i < Num_Connections; ++i) {
			EXPECT_EQ(PeerConnectResult::Accepted, state.Results[i]);
			EXPECT_EQ(context.ClientKeyPair.publicKey(), state.ClientKeys[i]);
		}

		EXPECT_EQ(Num_Connections, context.pConnector->numActiveConnections());
	}

	namespace {
		using ConnectingSocketHandler = consumer<std::shared_ptr<ionet::PacketSocket>&>;

		void RunConnectingSocketTest(const ConnectorTestContext& context, const ConnectingSocketHandler& handler) {
			std::atomic<size_t> numCallbacks(0);

			// Act: start a server verify operation that the client does not respond to
			//      (use a result shared_ptr so that the accept callback is valid even after this function returns)
			auto pResult = std::make_shared<PeerConnectResult>(static_cast<PeerConnectResult>(-1));
			std::shared_ptr<ionet::PacketSocket> pServerSocket;
			test::SpawnPacketServerWork(context.Service, [&, pResult](const auto& pSocket) {
				pServerSocket = pSocket;
				context.pConnector->accept(pSocket, [&, pResult](auto acceptResult, const auto&) {
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
			WAIT_FOR_VALUE(2u, numCallbacks);

			// Assert: the server accept callback was neved called
			EXPECT_EQ(static_cast<PeerConnectResult>(-1), *pResult);

			// - call the test handler
			handler(pServerSocket);
		}
	}

	TEST(TEST_CLASS, VerifyingConnectionIsIncludedInNumActiveConnections) {
		// Act:
		ConnectorTestContext context;
		RunConnectingSocketTest(context, [&](const auto&) {
			// Assert: the verifying connection is active
			EXPECT_EQ(1u, context.pConnector->numActiveConnections());
		});
	}

	TEST(TEST_CLASS, ShutdownClosesVerifyingSocket) {
		// Act:
		ConnectorTestContext context;
		RunConnectingSocketTest(context, [&](const auto& pServerSocket) {
			// Act: shutdown the connector
			context.pConnector->shutdown();

			// Assert: the server socket was closed
			EXPECT_FALSE(test::IsSocketOpen(*pServerSocket));
			EXPECT_EQ(0u, context.pConnector->numActiveConnections());
		});
	}

	namespace {
		bool RunTimeoutTestIteration(const ConnectorTestContext& context) {
			bool isSocketAccepted = false;
			RunConnectedSocketTest(context, [&](auto result, const auto& clientKey, auto& pServerSocket, const auto&) {
				// Retry: if the socket was accepted too quickly (before it timed out)
				isSocketAccepted = PeerConnectResult::Accepted == result;
				if (isSocketAccepted) {
					CATAPULT_LOG(warning) << "Socket was accepted before timeout";
					return;
				}

				// Assert: the server socket should have timed out and was closed
				EXPECT_EQ(PeerConnectResult::Timed_Out, result);
				EXPECT_EQ(Key(), clientKey);
				EXPECT_FALSE(test::IsSocketOpen(*pServerSocket));

				// Act: wait for all other references to drop to zero and release the last reference
				test::WaitForUnique(pServerSocket, "pServerSocket");
				pServerSocket.reset();

				// Assert: the active connection count drops to zero
				EXPECT_EQ(0u, context.pConnector->numActiveConnections());
			});

			return !isSocketAccepted;
		}

		void RunTimeoutTest(const ConnectorTestContext& context) {
			// Assert: non-deterministic because a socket could be accepted before it times out
			test::RunNonDeterministicTest("Timeout", [&]() {
				return RunTimeoutTestIteration(context);
			});
		}
	}

	TEST(TEST_CLASS, TimeoutClosesVerifyingSocket) {
		// Act:
		ConnectionSettings settings;
		settings.Timeout = utils::TimeSpan::FromMilliseconds(1);
		ConnectorTestContext context(settings);
		RunTimeoutTest(context);
	}
}}
