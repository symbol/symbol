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
#include "catapult/ionet/PacketSocket.h"
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
			explicit ConnectorTestContext(const ConnectionSettings& settings = test::CreateConnectionSettings())
					: ServerPublicKey(test::GenerateRandomByteArray<Key>())
					, ClientPublicKey(test::GenerateRandomByteArray<Key>())
					, pPool(test::CreateStartedIoThreadPool())
					, IoContext(pPool->ioContext())
					, pConnector(CreateClientConnector(*pPool, ServerPublicKey, settings))
			{}

			~ConnectorTestContext() {
				pConnector->shutdown();
				test::WaitForUnique(pConnector, "pConnector");
				pPool->join();
			}

		public:
			Key ServerPublicKey;
			Key ClientPublicKey;
			std::unique_ptr<thread::IoThreadPool> pPool;
			boost::asio::io_context& IoContext;
			std::shared_ptr<ClientConnector> pConnector;
		};

		// endregion

		// region MultiConnectionState

		struct MultiConnectionState {
			std::vector<PeerConnectCode> Codes;
			std::vector<Key> ClientPublicKeys;
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

					ionet::PacketSocketInfo socketInfo("", context.ClientPublicKey, pSocket);
					context.pConnector->accept(socketInfo, [&](auto acceptConnectCode, const auto&, const auto& publicKey) {
						state.Codes.push_back(acceptConnectCode);
						state.ClientPublicKeys.push_back(publicKey);
						++numCallbacks;
					});
				});

				test::SpawnPacketClientWork(context.IoContext, [&](const auto& pSocket) {
					state.ClientSockets.push_back(pSocket);
					++numCallbacks;
				});

				// - wait for both connections to complete
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
			Key ClientPublicKey;
			std::atomic<size_t> NumCallbacks;
		};

		void AcceptAndCapture(
				ClientConnector& connector,
				const std::shared_ptr<ionet::PacketSocket>& pSocket,
				AcceptCallbackParams& capture) {
			connector.accept(test::CreatePacketSocketInfo(pSocket), [&capture](
					auto acceptConnectCode,
					const auto& pVerifiedSocket,
					const auto& publicKey) {
				capture.Code = acceptConnectCode;
				capture.pClientSocket = pVerifiedSocket;
				capture.ClientPublicKey = publicKey;
				++capture.NumCallbacks;
			});
		}

		// endregion

		// region asserts

		void AssertFailed(PeerConnectCode expectedCode, const AcceptCallbackParams& capture) {
			EXPECT_EQ(expectedCode, capture.Code);
			EXPECT_FALSE(!!capture.pClientSocket);
			EXPECT_EQ(Key(), capture.ClientPublicKey);
		}

		// endregion
	}

	// region constructor

	TEST(TEST_CLASS, CanCreateConnectorWithDefaultName) {
		// Act:
		auto pPool = test::CreateStartedIoThreadPool();
		auto pConnector = CreateClientConnector(*pPool, Key(), ConnectionSettings());

		// Assert:
		EXPECT_EQ(0u, pConnector->numActiveConnections());
		EXPECT_EQ("", pConnector->name());
	}

	TEST(TEST_CLASS, CanCreateConnectorWithCustomName) {
		// Act:
		auto pPool = test::CreateStartedIoThreadPool();
		auto pConnector = CreateClientConnector(*pPool, Key(), ConnectionSettings(), "Crazy Amazing");

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

	// endregion

	// region self connections

	TEST(TEST_CLASS, AcceptFailsOnSelfConnection_WhenSelfConnectionsDisallowed) {
		// Arrange: prohibit self connections
		ConnectionSettings settings;
		settings.AllowIncomingSelfConnections = false;

		// Act: establish a single (self) connection
		ConnectorTestContext context(settings);
		context.ClientPublicKey = context.ServerPublicKey;
		auto state = SetupMultiConnectionTest(context, 1);

		// Assert: the accept should have failed and the connection should be closed
		EXPECT_EQ(PeerConnectCode::Self_Connection_Error, state.Codes.back());
		EXPECT_EQ(Key(), state.ClientPublicKeys.back());
		EXPECT_EQ(0u, context.pConnector->numActiveConnections());
	}

	TEST(TEST_CLASS, AcceptSucceedsOnSelfConnection_WhenSelfConnectionsAllowed) {
		// Act: establish a single (self) connection
		ConnectorTestContext context;
		context.ClientPublicKey = context.ServerPublicKey;
		auto state = SetupMultiConnectionTest(context, 1);

		// Assert: the accept should have succeeded and the connection should still be active
		EXPECT_EQ(PeerConnectCode::Accepted, state.Codes.back());
		EXPECT_EQ(context.ClientPublicKey, state.ClientPublicKeys.back());
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
			handler(state.Codes.back(), state.ClientPublicKeys.back(), state.ServerSockets.back(), state.ClientSockets.back());
		}
	}

	TEST(TEST_CLASS, AcceptSucceedsOnVerifySuccess) {
		// Act:
		ConnectorTestContext context;
		RunConnectedSocketTest(context, [&](auto connectCode, const auto& clientPublicKey, const auto&, const auto&) {
			// Assert: the verification should have succeeded and the connection should be active
			EXPECT_EQ(PeerConnectCode::Accepted, connectCode);
			EXPECT_EQ(context.ClientPublicKey, clientPublicKey);
			EXPECT_EQ(1u, context.pConnector->numActiveConnections());
		});
	}

	TEST(TEST_CLASS, ShutdownClosesAcceptedSocket) {
		// Act:
		ConnectorTestContext context;
		RunConnectedSocketTest(context, [&](auto, const auto&, const auto& pServerSocket, const auto&) {
			// Act: shutdown the connector
			context.pConnector->shutdown();
			test::WaitForClosedSocket(*pServerSocket);

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
			EXPECT_EQ(context.ClientPublicKey, state.ClientPublicKeys[i]);
		}

		EXPECT_EQ(Num_Connections, context.pConnector->numActiveConnections());
	}

	// endregion
}}
