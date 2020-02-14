/**
*** Copyright (c) 2016-present,
*** Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp. All rights reserved.
***
*** This file is part of Catapult.
***
*** Catapult is free software: you can redistribute it and/or modify
*** it under the terms of the GNU Lesser General Public License as published by
*** the Free Software Foundation, either version 3 of the License, or
*** (at your option) any later version.
***
*** Catapult is distributed in the hope that it will be useful,
*** but WITHOUT ANY WARRANTY; without even the implied warranty of
*** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*** GNU Lesser General Public License for more details.
***
*** You should have received a copy of the GNU Lesser General Public License
*** along with Catapult. If not, see <http://www.gnu.org/licenses/>.
**/

#include "catapult/net/ClientConnector.h"
#include "catapult/crypto/KeyPair.h"
#include "catapult/ionet/PacketSocket.h"
#include "catapult/net/VerifyPeer.h"
#include "catapult/thread/IoThreadPool.h"
#include "tests/test/core/ThreadPoolTestUtils.h"
#include "tests/test/net/SocketTestUtils.h"
#include "tests/test/nodeps/KeyTestUtils.h"

namespace catapult { namespace net {

#define TEST_CLASS ClientConnectorTests

	namespace {
		// region ConnectorTestContext

		struct ConnectorTestContext {
		public:
			explicit ConnectorTestContext(const ConnectionSettings& settings = ConnectionSettings())
					: ServerKeyPair(test::GenerateKeyPair())
					, ClientKeyPair(test::GenerateKeyPair())
					, pPool(test::CreateStartedIoThreadPool())
					, IoContext(pPool->ioContext())
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
			std::shared_ptr<thread::IoThreadPool> pPool;
			boost::asio::io_context& IoContext;
			std::shared_ptr<ClientConnector> pConnector;
		};

		// endregion

		// region MultiConnectionState

		struct MultiConnectionState {
			std::vector<PeerConnectCode> Codes;
			std::vector<Key> ClientKeys;
			std::vector<std::shared_ptr<ionet::PacketSocket>> ServerSockets;
			std::vector<std::shared_ptr<ionet::PacketSocket>> ClientSockets;
		};

		MultiConnectionState SetupMultiConnectionTest(const ConnectorTestContext& context, size_t numConnections) {
			// Act: start multiple server and client verify operations
			MultiConnectionState state;
			test::TcpAcceptor acceptor(context.IoContext);
			for (auto i = 0u; i < numConnections; ++i) {
				std::atomic<size_t> numCallbacks(0);
				test::SpawnPacketServerWork(acceptor, [&](const auto& pSocket) {
					state.ServerSockets.push_back(pSocket);
					context.pConnector->accept(pSocket, [&](auto result, const auto&, const auto& key) {
						state.Codes.push_back(result);
						state.ClientKeys.push_back(key);
						++numCallbacks;
					});
				});

				test::SpawnPacketClientWork(context.IoContext, [&](const auto& pSocket) {
					state.ClientSockets.push_back(pSocket);
					auto serverPeerInfo = VerifiedPeerInfo{ context.ServerKeyPair.publicKey(), ionet::ConnectionSecurityMode::None };
					VerifyServer(pSocket, serverPeerInfo, context.ClientKeyPair, [&](auto, const auto&) {
						++numCallbacks;
					});
				});

				// - wait for both verifications to complete
				WAIT_FOR_VALUE(2u, numCallbacks);
			}

			return state;
		}

		// endregion

		// region AcceptCallbackParams

		struct AcceptCallbackParams {
		public:
			AcceptCallbackParams() : NumCallbacks(0)
			{}

		public:
			PeerConnectCode Code;
			std::shared_ptr<ionet::PacketSocket> pClientSocket;
			Key ClientKey;
			std::atomic<size_t> NumCallbacks;
		};

		void AcceptAndCapture(
				ClientConnector& connector,
				const std::shared_ptr<ionet::PacketSocket>& pSocket,
				AcceptCallbackParams& capture) {
			connector.accept(pSocket, [&capture](auto acceptConnectCode, const auto& pVerifiedSocket, const auto& key) {
				capture.Code = acceptConnectCode;
				capture.pClientSocket = pVerifiedSocket;
				capture.ClientKey = key;
				++capture.NumCallbacks;
			});
		}

		// endregion

		// region asserts

		void AssertFailed(PeerConnectCode expectedCode, const AcceptCallbackParams& capture) {
			EXPECT_EQ(expectedCode, capture.Code);
			EXPECT_FALSE(!!capture.pClientSocket);
			EXPECT_EQ(Key(), capture.ClientKey);
		}

		// endregion
	}

	// region constructor

	TEST(TEST_CLASS, CanCreateConnectorWithDefaultName) {
		// Act:
		auto pPool = test::CreateStartedIoThreadPool();
		auto pConnector = CreateClientConnector(std::move(pPool), test::GenerateKeyPair(), ConnectionSettings());

		// Assert:
		EXPECT_EQ(0u, pConnector->numActiveConnections());
		EXPECT_EQ("", pConnector->name());
	}

	TEST(TEST_CLASS, CanCreateConnectorWithCustomName) {
		// Act:
		auto pPool = test::CreateStartedIoThreadPool();
		auto pConnector = CreateClientConnector(std::move(pPool), test::GenerateKeyPair(), ConnectionSettings(), "Crazy Amazing");

		// Assert:
		EXPECT_EQ(0u, pConnector->numActiveConnections());
		EXPECT_EQ("Crazy Amazing", pConnector->name());
	}

	// endregion

	// region errors

	TEST(TEST_CLASS, AcceptFailsOnAcceptError) {
		// Arrange:
		ConnectorTestContext context;

		// Act: on an accept error, the server will pass nullptr
		AcceptCallbackParams capture;
		AcceptAndCapture(*context.pConnector, nullptr, capture);

		// Assert:
		AssertFailed(PeerConnectCode::Socket_Error, capture);
		EXPECT_EQ(0u, context.pConnector->numActiveConnections());
	}

	TEST(TEST_CLASS, AcceptFailsOnVerifyError) {
		// Arrange:
		ConnectorTestContext context;

		// Act: start a server and client verify operation
		AcceptCallbackParams capture;
		test::SpawnPacketServerWork(context.IoContext, [&](const auto& pSocket) {
			AcceptAndCapture(*context.pConnector, pSocket, capture);
		});

		test::SpawnPacketClientWork(context.IoContext, [&](const auto& pSocket) {
			// - trigger a verify error by closing the socket without responding
			pSocket->close();
			++capture.NumCallbacks;
		});

		// - wait for both callbacks to complete and the connection to close
		WAIT_FOR_VALUE(2u, capture.NumCallbacks);
		WAIT_FOR_ZERO_EXPR(context.pConnector->numActiveConnections());

		// Assert: the verification should have failed and all connections should have been destroyed
		AssertFailed(PeerConnectCode::Verify_Error, capture);
		EXPECT_EQ(0u, context.pConnector->numActiveConnections());
	}

	// endregion

	// region self connections

	TEST(TEST_CLASS, AcceptFailsOnSelfConnection_WhenSelfConnectionsDisallowed) {
		// Arrange: prohibit self connections
		ConnectionSettings settings;
		settings.AllowIncomingSelfConnections = false;

		// Act: establish a single (self) connection
		ConnectorTestContext context(settings);
		context.ServerKeyPair = test::CopyKeyPair(context.ClientKeyPair);
		auto state = SetupMultiConnectionTest(context, 1);

		// Assert: the accept should have failed but the connection should still be active (kept alive by `state`)
		EXPECT_EQ(PeerConnectCode::Self_Connection_Error, state.Codes.back());
		EXPECT_EQ(Key(), state.ClientKeys.back());
		EXPECT_EQ(1u, context.pConnector->numActiveConnections());
	}

	TEST(TEST_CLASS, AcceptSucceedsOnSelfConnection_WhenSelfConnectionsAllowed) {
		// Act: establish a single (self) connection
		ConnectorTestContext context;
		context.ServerKeyPair = test::CopyKeyPair(context.ClientKeyPair);
		auto state = SetupMultiConnectionTest(context, 1);

		// Assert: the accept should have succeeded and the connection should still be active
		EXPECT_EQ(PeerConnectCode::Accepted, state.Codes.back());
		EXPECT_EQ(context.ClientKeyPair.publicKey(), state.ClientKeys.back());
		EXPECT_EQ(1u, context.pConnector->numActiveConnections());
	}

	// endregion

	// region connected socket

	namespace {
		using ConnectedSocketHandler = consumer<
				PeerConnectCode,
				const Key&,
				std::shared_ptr<ionet::PacketSocket>&,
				std::shared_ptr<ionet::PacketSocket>&>;

		void RunConnectedSocketTest(const ConnectorTestContext& context, const ConnectedSocketHandler& handler) {
			// Act: establish a single connection
			auto state = SetupMultiConnectionTest(context, 1);

			// Assert: call the handler
			handler(state.Codes.back(), state.ClientKeys.back(), state.ServerSockets.back(), state.ClientSockets.back());
		}
	}

	TEST(TEST_CLASS, AcceptSucceedsOnVerifySuccess) {
		// Act:
		ConnectorTestContext context;
		RunConnectedSocketTest(context, [&](auto connectCode, const auto& clientKey, const auto&, const auto&) {
			// Assert: the verification should have succeeded and the connection should be active
			EXPECT_EQ(PeerConnectCode::Accepted, connectCode);
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
		constexpr auto Num_Connections = 5u;
		ConnectorTestContext context;
		auto state = SetupMultiConnectionTest(context, Num_Connections);

		// Assert: all connections are active
		ASSERT_EQ(Num_Connections, state.Codes.size());
		for (auto i = 0u; i < Num_Connections; ++i) {
			EXPECT_EQ(PeerConnectCode::Accepted, state.Codes[i]);
			EXPECT_EQ(context.ClientKeyPair.publicKey(), state.ClientKeys[i]);
		}

		EXPECT_EQ(Num_Connections, context.pConnector->numActiveConnections());
	}

	// endregion

	// region connecting socket

	namespace {
		using ConnectingSocketHandler = consumer<std::shared_ptr<ionet::PacketSocket>&>;

		void RunConnectingSocketTest(const ConnectorTestContext& context, const ConnectingSocketHandler& handler) {
			std::atomic<size_t> numCallbacks(0);

			// Act: start a server verify operation that the client does not respond to
			//      (use a result shared_ptr so that the accept callback is valid even after this function returns)
			auto pConnectCode = std::make_shared<PeerConnectCode>(static_cast<PeerConnectCode>(-1));
			std::shared_ptr<ionet::PacketSocket> pServerSocket;
			test::SpawnPacketServerWork(context.IoContext, [&, pConnectCode](const auto& pSocket) {
				pServerSocket = pSocket;
				context.pConnector->accept(pSocket, [&, pConnectCode](auto acceptConnectCode, const auto&, const auto&) {
					// note that this is not expected to get called until shutdown because the client doesn't read or write any data
					*pConnectCode = acceptConnectCode;
				});
				++numCallbacks;
			});

			std::shared_ptr<ionet::PacketSocket> pClientSocket;
			test::SpawnPacketClientWork(context.IoContext, [&](const auto& pSocket) {
				pClientSocket = pSocket;
				++numCallbacks;
			});

			// - wait for the initial work to complete
			WAIT_FOR_VALUE(2u, numCallbacks);

			// Assert: the server accept callback was neved called
			EXPECT_EQ(static_cast<PeerConnectCode>(-1), *pConnectCode);

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

	// endregion

	// region timeout

	namespace {
		bool RunTimeoutTestIteration(const ConnectorTestContext& context) {
			bool isSocketAccepted = false;
			RunConnectedSocketTest(context, [&](auto connectCode, const auto& clientKey, auto& pServerSocket, const auto&) {
				// Retry: if the socket was accepted too quickly (before it timed out)
				isSocketAccepted = PeerConnectCode::Accepted == connectCode;
				if (isSocketAccepted) {
					CATAPULT_LOG(warning) << "Socket was accepted before timeout";
					return;
				}

				// Assert: the server socket should have timed out and was closed
				EXPECT_EQ(PeerConnectCode::Timed_Out, connectCode);
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
		settings.Timeout = utils::TimeSpan::FromMilliseconds(0);
		ConnectorTestContext context(settings);
		RunTimeoutTest(context);
	}

	// endregion
}}
