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

#include "catapult/net/ServerConnector.h"
#include "catapult/crypto/OpensslKeyUtils.h"
#include "catapult/ionet/Node.h"
#include "catapult/ionet/PacketSocket.h"
#include "catapult/thread/IoThreadPool.h"
#include "tests/test/core/ThreadPoolTestUtils.h"
#include "tests/test/net/CertificateLocator.h"
#include "tests/test/net/ClientSocket.h"
#include "tests/test/net/NodeTestUtils.h"
#include "tests/test/net/SocketTestUtils.h"
#include "tests/test/nodeps/KeyTestUtils.h"

namespace catapult { namespace net {

#define TEST_CLASS ServerConnectorTests

	namespace {
		// region ConnectorTestContext

		struct ConnectorTestContext {
		public:
			explicit ConnectorTestContext(const Key& publicKey) : ConnectorTestContext(test::CreateConnectionSettings(publicKey)) {
				ServerPublicKey = publicKey;
			}

			explicit ConnectorTestContext(const ConnectionSettings& settings = test::CreateConnectionSettings())
					: ServerPublicKey(test::GenerateRandomByteArray<Key>())
					, ClientPublicKey(test::GenerateRandomByteArray<Key>())
					, pPool(test::CreateStartedIoThreadPool())
					, IoContext(pPool->ioContext())
					, pConnector(CreateServerConnector(*pPool, ClientPublicKey, settings))
			{}

			~ConnectorTestContext() {
				pConnector->shutdown();
				test::WaitForUnique(pConnector, "pConnector");

				CATAPULT_LOG(debug) << "waiting for pool in ConnectorTestContext to drain";
				pPool->join();
			}

		public:
			Key ServerPublicKey;
			Key ClientPublicKey;
			std::unique_ptr<thread::IoThreadPool> pPool;
			boost::asio::io_context& IoContext;
			std::shared_ptr<ServerConnector> pConnector;

		public:
			ionet::Node serverNode() const {
				return test::CreateLocalHostNode(ServerPublicKey);
			}

			void waitForActiveConnections(uint32_t numConnections) const {
				WAIT_FOR_VALUE_EXPR(numConnections, pConnector->numActiveConnections());
			}
		};

		// endregion

		// region MultiConnectionState

		struct MultiConnectionState {
			std::vector<PeerConnectCode> Codes;
			std::vector<std::shared_ptr<ionet::PacketSocket>> ServerSockets;
			std::vector<ionet::PacketSocketInfo> ClientSocketInfos;
		};

		MultiConnectionState SetupMultiConnectionTest(const ConnectorTestContext& context, size_t numConnections) {
			// Act: start multiple server and client verify operations
			MultiConnectionState state;
			test::TcpAcceptor acceptor(context.IoContext);
			for (auto i = 0u; i < numConnections; ++i) {
				std::atomic<size_t> numCallbacks(0);
				test::SpawnPacketServerWork(acceptor, [&](const auto& pSocket) {
					state.ServerSockets.push_back(pSocket);
					++numCallbacks;
				});

				context.pConnector->connect(context.serverNode(), [&](auto connectResult, const auto& connectedSocketInfo) {
					state.Codes.push_back(connectResult);
					state.ClientSocketInfos.push_back(connectedSocketInfo);
					++numCallbacks;
				});

				// - wait for both connections to complete
				WAIT_FOR_VALUE(2u, numCallbacks);
			}

			return state;
		}

		// endregion
	}

	// region constructor

	TEST(TEST_CLASS, CanCreateConnectorWithDefaultName) {
		// Act:
		auto pPool = test::CreateStartedIoThreadPool();
		auto pConnector = CreateServerConnector(*pPool, Key(), ConnectionSettings());

		// Assert:
		EXPECT_EQ(0u, pConnector->numActiveConnections());
		EXPECT_EQ("", pConnector->name());
	}

	TEST(TEST_CLASS, CanCreateConnectorWithCustomName) {
		// Act:
		auto pPool = test::CreateStartedIoThreadPool();
		auto pConnector = CreateServerConnector(*pPool, Key(), ConnectionSettings(), "Crazy Amazing");

		// Assert:
		EXPECT_EQ(0u, pConnector->numActiveConnections());
		EXPECT_EQ("Crazy Amazing", pConnector->name());
	}

	// endregion

	// region errors

	TEST(TEST_CLASS, ConnectFailsOnSelfConnectionError_WhenSelfConnectionsDisallowed) {
		// Arrange:
		ConnectorTestContext context;
		context.ServerPublicKey = context.ClientPublicKey;
		std::atomic<size_t> numCallbacks(0);

		// Act: try to connect to a server with same public key
		PeerConnectCode code;
		ionet::PacketSocketInfo socketInfo;
		context.pConnector->connect(context.serverNode(), [&](auto connectCode, const auto& connectedSocketInfo) {
			code = connectCode;
			socketInfo = connectedSocketInfo;
			++numCallbacks;
		});
		WAIT_FOR_ONE(numCallbacks);

		// Assert:
		EXPECT_EQ(PeerConnectCode::Self_Connection_Error, code);
		test::AssertEmpty(socketInfo);
		EXPECT_EQ(0u, context.pConnector->numActiveConnections());
	}

	TEST(TEST_CLASS, ConnectDoesNotFailOnSelfConnectionError_WhenSelfConnectionsAllowed) {
		// Arrange: allow self connections
		auto settings = test::CreateConnectionSettings();
		settings.AllowOutgoingSelfConnections = true;

		ConnectorTestContext context(settings);
		context.ServerPublicKey = context.ClientPublicKey;
		std::atomic<size_t> numCallbacks(0);

		// Act: try to connect to a server with same public key
		PeerConnectCode code;
		ionet::PacketSocketInfo socketInfo;
		context.pConnector->connect(context.serverNode(), [&](auto connectCode, const auto& connectedSocketInfo) {
			code = connectCode;
			socketInfo = connectedSocketInfo;
			++numCallbacks;
		});
		WAIT_FOR_ONE(numCallbacks);

		// Assert: Self_Connection_Error has priority over Socket_Error
		EXPECT_EQ(PeerConnectCode::Socket_Error, code);
		test::AssertEmpty(socketInfo);
		EXPECT_EQ(0u, context.pConnector->numActiveConnections());
	}

	TEST(TEST_CLASS, ConnectFailsOnConnectError) {
		// Arrange:
		ConnectorTestContext context;
		std::atomic<size_t> numCallbacks(0);

		// Act: try to connect to a server that isn't running
		PeerConnectCode code;
		ionet::PacketSocketInfo socketInfo;
		context.pConnector->connect(context.serverNode(), [&](auto connectCode, const auto& connectedSocketInfo) {
			code = connectCode;
			socketInfo = connectedSocketInfo;
			++numCallbacks;
		});
		WAIT_FOR_ONE(numCallbacks);

		// Assert:
		EXPECT_EQ(PeerConnectCode::Socket_Error, code);
		test::AssertEmpty(socketInfo);
		EXPECT_EQ(0u, context.pConnector->numActiveConnections());
	}

	// endregion

	// region connected socket

	namespace {
		using ResultServerClientHandler = consumer<
			PeerConnectCode,
			const std::shared_ptr<ionet::PacketSocket>&,
			const ionet::PacketSocketInfo&>;

		void RunConnectedSocketTest(const ConnectorTestContext& context, const ResultServerClientHandler& handler) {
			// Act: establish a single connection
			auto state = SetupMultiConnectionTest(context, 1);

			// Assert: call the handler
			handler(state.Codes.back(), state.ServerSockets.back(), state.ClientSocketInfos.back());
		}
	}

	TEST(TEST_CLASS, ConnectFailsOnVerifyError) {
		// Arrange:
		ConnectorTestContext context(test::GenerateRandomByteArray<Key>());
		context.ServerPublicKey = test::GenerateRandomByteArray<Key>();

		// Act:
		RunConnectedSocketTest(context, [&](auto connectCode, const auto&, const auto& clientSocketInfo) {
			// Assert: the verification should have failed
			EXPECT_EQ(PeerConnectCode::Verify_Error, connectCode);
			test::AssertEmpty(clientSocketInfo);
			EXPECT_EQ(0u, context.pConnector->numActiveConnections());
		});
	}

	TEST(TEST_CLASS, ConnectSucceedsOnVerifySuccess) {
		// Arrange:
		ConnectorTestContext context(test::GenerateRandomByteArray<Key>());

		// Act:
		RunConnectedSocketTest(context, [&](auto connectCode, const auto&, const auto& clientSocketInfo) {
			// Assert: the verification should have succeeded and the connection should be active
			EXPECT_EQ(PeerConnectCode::Accepted, connectCode);
			EXPECT_EQ(1u, context.pConnector->numActiveConnections());
			EXPECT_TRUE(!!clientSocketInfo.socket());
			EXPECT_EQ("127.0.0.1", clientSocketInfo.host());
			EXPECT_EQ(context.ServerPublicKey, clientSocketInfo.publicKey());
		});
	}

	TEST(TEST_CLASS, ShutdownClosesConnectedSocket) {
		// Arrange:
		ConnectorTestContext context(test::GenerateRandomByteArray<Key>());

		// Act:
		RunConnectedSocketTest(context, [&context](auto, const auto&, const auto& clientSocketInfo) {
			// Act: shutdown the connector
			context.pConnector->shutdown();
			test::WaitForClosedSocket(*clientSocketInfo.socket());

			// Assert: the client socket was closed
			EXPECT_FALSE(test::IsSocketOpen(*clientSocketInfo.socket()));
			EXPECT_EQ(0u, context.pConnector->numActiveConnections());
		});
	}

	TEST(TEST_CLASS, CanManageMultipleConnections) {
		// Arrange: establish multiple connections
		constexpr auto Num_Connections = 5u;
		ConnectorTestContext context(test::GenerateRandomByteArray<Key>());

		// Act:
		auto state = SetupMultiConnectionTest(context, Num_Connections);

		// Assert: all connections are active (notice that connect is passed server information)
		EXPECT_EQ(Num_Connections, state.Codes.size());
		for (auto i = 0u; i < Num_Connections; ++i) {
			EXPECT_EQ(PeerConnectCode::Accepted, state.Codes[i]);
			EXPECT_TRUE(!!state.ClientSocketInfos[i].socket());
			EXPECT_EQ("127.0.0.1", state.ClientSocketInfos[i].host());
			EXPECT_EQ(context.ServerPublicKey, state.ClientSocketInfos[i].publicKey());
		}

		EXPECT_EQ(Num_Connections, context.pConnector->numActiveConnections());
	}

	// endregion

	// region timeout

	namespace {
		bool RunTimeoutTestIteration(const ConnectionSettings& settings, size_t numDesiredActiveConnections) {
			// Arrange:
			ConnectorTestContext context(settings);
			std::atomic<size_t> numCallbacks(0);
			std::atomic<size_t> numDummyConnections(0);

			// Act: start a verify operation that the server does not respond to
			// - server: accept a single connection
			CATAPULT_LOG(debug) << "starting async accept";
			test::TcpAcceptor acceptor(context.IoContext);
			std::shared_ptr<ionet::PacketSocket> pServerSocket;
			boost::asio::post(acceptor.strand(), [&context, &acceptor = acceptor.get(), &numCallbacks, &pServerSocket]() {
				ionet::Accept(context.IoContext, acceptor, test::CreatePacketSocketOptions(), [&numCallbacks, &pServerSocket](
						const auto& socketInfo) {
					pServerSocket = socketInfo.socket();
					++numCallbacks;
				});
			});

			// - client: start a connection to the server
			PeerConnectCode code;
			size_t numActiveConnections = 0;
			ionet::PacketSocketInfo clientSocketInfo;
			context.pConnector->connect(context.serverNode(), [&](auto connectCode, const auto& connectedSocketInfo) {
				// - note that any active connections will not be destroyed until the completion of this callback
				numActiveConnections = context.pConnector->numActiveConnections();

				code = connectCode;
				clientSocketInfo = connectedSocketInfo;
				++numCallbacks;

				// - if the connect callback is called first, the request likely timed out during connect
				if (numCallbacks < 2) {
					++numDummyConnections;

					// - cancel all outstanding acceptor operations to allow the server to shutdown
					CATAPULT_LOG(debug) << "cancelling outstanding acceptor operations";
					boost::asio::post(acceptor.strand(), [&acceptor = acceptor.get()]() {
						acceptor.cancel();
					});
				}
			});

			// - wait for both callbacks to be called
			WAIT_FOR_VALUE(2u, numCallbacks);

			// Retry: if there are an unexpected number of connections or dummy connections
			if (numActiveConnections != numDesiredActiveConnections || numDummyConnections == numDesiredActiveConnections) {
				CATAPULT_LOG(warning)
						<< "unexpected number of connections " << numActiveConnections
						<< " or dummy connections " << numDummyConnections;
				return false;
			}

			// Assert: the client connect handler was called with a timeout and nullptr
			EXPECT_EQ(PeerConnectCode::Timed_Out, code);
			test::AssertEmpty(clientSocketInfo);

			// - wait for all connections to be destroyed
			context.waitForActiveConnections(0);
			return true;
		}

		void RunTimeoutTest(const ConnectionSettings& settings, size_t numDesiredActiveConnections) {
			// Assert: non-deterministic because a socket could connect before it times out and/or timeout in the
			//         wrong state (connecting vs verifying)
			test::RunNonDeterministicTest("Timeout", [&]() {
				return RunTimeoutTestIteration(settings, numDesiredActiveConnections);
			});
		}
	}

	TEST(TEST_CLASS, TimeoutClosesConnectingSocket) {
		// Arrange: timeout immediately (during connect where 0 active connections are expected)
		const auto Num_Expected_Active_Connections = 0;
		auto settings = test::CreateConnectionSettings();
		settings.Timeout = utils::TimeSpan::FromMilliseconds(0);

		// Assert:
		RunTimeoutTest(settings, Num_Expected_Active_Connections);
	}

	// endregion
}}
