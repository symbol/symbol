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
#include "catapult/crypto/KeyPair.h"
#include "catapult/ionet/Node.h"
#include "catapult/ionet/PacketSocket.h"
#include "catapult/net/VerifyPeer.h"
#include "catapult/thread/IoThreadPool.h"
#include "tests/test/core/ThreadPoolTestUtils.h"
#include "tests/test/net/ClientSocket.h"
#include "tests/test/net/NodeTestUtils.h"
#include "tests/test/net/SocketTestUtils.h"
#include "tests/test/nodeps/KeyTestUtils.h"

namespace catapult { namespace net {

#define TEST_CLASS ServerConnectorTests

	namespace {
		struct ConnectorTestContext {
		public:
			explicit ConnectorTestContext(const ConnectionSettings& settings = ConnectionSettings())
					: ServerKeyPair(test::GenerateKeyPair())
					, ClientKeyPair(test::GenerateKeyPair())
					, pPool(test::CreateStartedIoThreadPool())
					, IoContext(pPool->ioContext())
					, pConnector(CreateServerConnector(pPool, ClientKeyPair, settings))
			{}

			~ConnectorTestContext() {
				pConnector->shutdown();
				test::WaitForUnique(pConnector, "pConnector");

				CATAPULT_LOG(debug) << "waiting for pool in ConnectorTestContext to drain";
				pPool->join();
			}

		public:
			crypto::KeyPair ServerKeyPair;
			crypto::KeyPair ClientKeyPair;
			std::shared_ptr<thread::IoThreadPool> pPool;
			boost::asio::io_context& IoContext;
			std::shared_ptr<ServerConnector> pConnector;

		public:
			ionet::Node serverNode() const {
				return test::CreateLocalHostNode(ServerKeyPair.publicKey());
			}

			void waitForActiveConnections(uint32_t numConnections) const {
				WAIT_FOR_VALUE_EXPR(numConnections, pConnector->numActiveConnections());
			}
		};
	}

	TEST(TEST_CLASS, InitiallyNoConnectionsAreActive) {
		// Act:
		auto pPool = test::CreateStartedIoThreadPool();
		auto pConnector = CreateServerConnector(std::move(pPool), test::GenerateKeyPair(), ConnectionSettings());

		// Assert:
		EXPECT_EQ(0u, pConnector->numActiveConnections());
	}

	TEST(TEST_CLASS, ConnectFailsOnConnectError) {
		// Arrange:
		ConnectorTestContext context;
		std::atomic<size_t> numCallbacks(0);

		// Act: try to connect to a server that isn't running
		PeerConnectCode code;
		std::shared_ptr<ionet::PacketSocket> pSocket;
		context.pConnector->connect(context.serverNode(), [&](auto connectCode, const auto& pConnectedSocket) {
			code = connectCode;
			pSocket = pConnectedSocket;
			++numCallbacks;
		});
		WAIT_FOR_ONE(numCallbacks);

		// Assert:
		EXPECT_EQ(PeerConnectCode::Socket_Error, code);
		EXPECT_FALSE(!!pSocket);
		EXPECT_EQ(0u, context.pConnector->numActiveConnections());
	}

	TEST(TEST_CLASS, ConnectFailsOnVerifyError) {
		// Arrange:
		ConnectorTestContext context;
		std::atomic<size_t> numCallbacks(0);

		// Act: start a server and client verify operation
		PeerConnectCode code;
		test::SpawnPacketServerWork(context.IoContext, [&](const auto& pSocket) {
			// - trigger a verify error by closing the socket without responding
			pSocket->close();
			++numCallbacks;
		});

		std::shared_ptr<ionet::PacketSocket> pSocket;
		context.pConnector->connect(context.serverNode(), [&](auto connectCode, const auto& pConnectedSocket) {
			code = connectCode;
			pSocket = pConnectedSocket;
			++numCallbacks;
		});

		// - wait for both callbacks to complete and the connection to close
		WAIT_FOR_VALUE(2u, numCallbacks);
		context.waitForActiveConnections(0);

		// Assert: the verification should have failed and all connections should have been destroyed
		EXPECT_EQ(PeerConnectCode::Verify_Error, code);
		EXPECT_FALSE(!!pSocket);
		EXPECT_EQ(0u, context.pConnector->numActiveConnections());
	}

	namespace {
		struct MultiConnectionState {
			std::vector<PeerConnectCode> Codes;
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
					VerifyClient(pSocket, context.ServerKeyPair, ionet::ConnectionSecurityMode::None, [&](auto, const auto&) {
						++numCallbacks;
					});
				});

				context.pConnector->connect(context.serverNode(), [&](auto connectResult, const auto& pSocket) {
					state.Codes.push_back(connectResult);
					state.ClientSockets.push_back(pSocket);
					++numCallbacks;
				});

				// - wait for both verifications to complete
				WAIT_FOR_VALUE(2u, numCallbacks);
			}

			return state;
		}

		using ResultServerClientHandler = consumer<
				PeerConnectCode,
				std::shared_ptr<ionet::PacketSocket>&,
				std::shared_ptr<ionet::PacketSocket>&>;

		void RunConnectedSocketTest(const ConnectorTestContext& context, const ResultServerClientHandler& handler) {
			// Act: establish a single connection
			auto state = SetupMultiConnectionTest(context, 1);

			// Assert: call the handler
			handler(state.Codes.back(), state.ServerSockets.back(), state.ClientSockets.back());
		}
	}

	TEST(TEST_CLASS, ConnectSucceedsOnVerifySuccess) {
		// Act:
		ConnectorTestContext context;
		RunConnectedSocketTest(context, [&](auto connectCode, const auto&, const auto& pClientSocket) {
			// Assert: the verification should have succeeded and the connection should be active
			EXPECT_EQ(PeerConnectCode::Accepted, connectCode);
			EXPECT_EQ(1u, context.pConnector->numActiveConnections());
			EXPECT_TRUE(!!pClientSocket);
		});
	}

	TEST(TEST_CLASS, ShutdownClosesConnectedSocket) {
		// Act:
		ConnectorTestContext context;
		RunConnectedSocketTest(context, [&](auto, const auto&, const auto& pClientSocket) {
			// Act: shutdown the connector
			context.pConnector->shutdown();

			// Assert: the client socket was closed
			EXPECT_FALSE(test::IsSocketOpen(*pClientSocket));
			EXPECT_EQ(0u, context.pConnector->numActiveConnections());
		});
	}

	TEST(TEST_CLASS, CanManageMultipleConnections) {
		// Act: establish multiple connections
		constexpr auto Num_Connections = 5u;
		ConnectorTestContext context;
		auto state = SetupMultiConnectionTest(context, Num_Connections);

		// Assert: all connections are active
		EXPECT_EQ(Num_Connections, state.Codes.size());
		for (auto i = 0u; i < Num_Connections; ++i) {
			EXPECT_EQ(PeerConnectCode::Accepted, state.Codes[i]);
			EXPECT_TRUE(!!state.ClientSockets[i]);
		}

		EXPECT_EQ(Num_Connections, context.pConnector->numActiveConnections());
	}

	namespace {
		void RunConnectingSocketTest(const ConnectorTestContext& context, const action& handler) {
			std::atomic<size_t> numCallbacks(0);

			// Act: start a verify operation that the server does not respond to
			std::shared_ptr<ionet::PacketSocket> pServerSocket;
			test::SpawnPacketServerWork(context.IoContext, [&](const auto& pSocket) {
				pServerSocket = pSocket;
				++numCallbacks;
			});

			// - (use a result shared_ptr so that the connect callback is valid even after this function returns)
			auto pCode = std::make_shared<PeerConnectCode>(static_cast<PeerConnectCode>(-1));
			context.pConnector->connect(context.serverNode(), [&, pCode](auto connectCode, const auto&) {
				// note that this is not expected to get called until shutdown because the client doesn't read or write any data
				*pCode = connectCode;
			});

			// - wait for the initial work to complete and the connection to become active
			WAIT_FOR_ONE(numCallbacks);
			context.waitForActiveConnections(1);

			// Assert: the client connect handler was never called
			EXPECT_EQ(static_cast<PeerConnectCode>(-1), *pCode);

			// - call the test handler
			handler();
		}
	}

	TEST(TEST_CLASS, VerifyingConnectionIsIncludedInNumActiveConnections) {
		// Act:
		ConnectorTestContext context;
		RunConnectingSocketTest(context, [&]() {
			// Assert: the verifying connection is active
			EXPECT_EQ(1u, context.pConnector->numActiveConnections());
		});
	}

	TEST(TEST_CLASS, ShutdownClosesVerifyingSocket) {
		// Act:
		ConnectorTestContext context;
		RunConnectingSocketTest(context, [&]() {
			// Act: shutdown the connector
			context.pConnector->shutdown();

			// Assert: the verifying socket is no longer active
			EXPECT_EQ(0u, context.pConnector->numActiveConnections());
		});
	}

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
			auto serverSocket = boost::asio::ip::tcp::socket(context.IoContext);
			boost::asio::post(acceptor.strand(), [&acceptor = acceptor.get(), &numCallbacks, &serverSocket]() {
				acceptor.async_accept(serverSocket, [&numCallbacks](const auto& acceptEc) {
					CATAPULT_LOG(debug) << "async_accept completed with: " << acceptEc.message();
					++numCallbacks;
				});
			});

			// - client: start a connection to the server
			PeerConnectCode code;
			size_t numActiveConnections = 0;
			std::shared_ptr<ionet::PacketSocket> pClientSocket;
			context.pConnector->connect(context.serverNode(), [&](auto connectCode, const auto& pSocket) {
				// - note that any active connections will not be destroyed until the completion of this callback
				numActiveConnections = context.pConnector->numActiveConnections();

				code = connectCode;
				pClientSocket = pSocket;
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
			EXPECT_FALSE(!!pClientSocket);

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
		ConnectionSettings settings;
		settings.Timeout = utils::TimeSpan::FromMilliseconds(0);

		// Assert:
		RunTimeoutTest(settings, Num_Expected_Active_Connections);
	}

	TEST(TEST_CLASS, TimeoutClosesVerifyingSocket) {
		// Arrange: timeout with some delay (during verify where 1 active connection is expected)
		const auto Num_Expected_Active_Connections = 1;
		ConnectionSettings settings;
		settings.Timeout = utils::TimeSpan::FromMilliseconds(50);

		// Assert:
		RunTimeoutTest(settings, Num_Expected_Active_Connections);
	}

	// endregion

	// region security mode

	namespace {
		struct SecurityModeTestState {
			PeerConnectCode ConnectCode;
			std::shared_ptr<ionet::PacketSocket> pServerSocket;
			std::shared_ptr<ionet::PacketSocket> pConnectedClientSocket;
		};

		template<typename TAction>
		void RunSecurityModeTest(const ConnectionSettings& settings, bool shouldSimulateError, TAction action) {
			// Arrange:
			ConnectorTestContext context(settings);
			test::TcpAcceptor acceptor(context.IoContext);
			auto pNumCallbacks = std::make_shared<std::atomic<size_t>>(0);

			// - make a single connection with custom settings
			SecurityModeTestState state;
			test::SpawnPacketServerWork(acceptor, [&, pNumCallbacks](const auto& pSocket) {
				state.pServerSocket = pSocket;
				VerifyClient(pSocket, context.ServerKeyPair, settings.IncomingSecurityModes, [&, pNumCallbacks](auto, const auto&) {
					++*pNumCallbacks;

					// - simulate an error by having the server close the socket
					if (shouldSimulateError)
						state.pServerSocket->close();
				});
			});

			context.pConnector->connect(context.serverNode(), [&, pNumCallbacks](auto connectCode, const auto& pSocket) {
				state.ConnectCode = connectCode;
				state.pConnectedClientSocket = pSocket;
				++*pNumCallbacks;
			});

			// - wait for the connection to be established
			WAIT_FOR_VALUE(2u, *pNumCallbacks);

			// Act: + Assert:
			action(state);
		}

		void AssertWrittenPacketType(
				const SecurityModeTestState& state,
				ionet::PacketType expectedPacketType,
				ionet::PacketType packetType) {
			// Arrange: write a regular packet
			auto pPacket = test::CreateRandomPacket(100, packetType);

			std::atomic_bool isPacketRead(false);
			ionet::PacketType readPacketType;
			state.pConnectedClientSocket->write(ionet::PacketPayload(pPacket), [&](auto) { // sign-decorating write
				state.pServerSocket->read([&](auto, const auto* pReadPacket) { // raw read
					readPacketType = pReadPacket ? pReadPacket->Type : ionet::PacketType::Undefined;
					isPacketRead = true;
				});
			});

			// - wait for the read to occur
			WAIT_FOR(isPacketRead);

			// Assert:
			EXPECT_EQ(expectedPacketType, readPacketType);
		}
	}

	TEST(TEST_CLASS, SecurityModeNoneDoesNotWriteSecurePackets) {
		// Arrange:
		ConnectionSettings settings;
		settings.OutgoingSecurityMode = ionet::ConnectionSecurityMode::None;
		settings.IncomingSecurityModes = ionet::ConnectionSecurityMode::None | ionet::ConnectionSecurityMode::Signed;

		// Act:
		RunSecurityModeTest(settings, false, [](const auto& state) {
			// Assert:
			EXPECT_EQ(PeerConnectCode::Accepted, state.ConnectCode);
			EXPECT_TRUE(!!state.pConnectedClientSocket);

			AssertWrittenPacketType(state, ionet::PacketType::Chain_Info, ionet::PacketType::Chain_Info);
		});
	}

	TEST(TEST_CLASS, SecurityModeSignedWritesSecurePackets) {
		// Arrange:
		ConnectionSettings settings;
		settings.OutgoingSecurityMode = ionet::ConnectionSecurityMode::Signed;
		settings.IncomingSecurityModes = ionet::ConnectionSecurityMode::None | ionet::ConnectionSecurityMode::Signed;

		// Act:
		RunSecurityModeTest(settings, false, [](const auto& state) {
			// Assert:
			EXPECT_EQ(PeerConnectCode::Accepted, state.ConnectCode);
			EXPECT_TRUE(!!state.pConnectedClientSocket);

			AssertWrittenPacketType(state, ionet::PacketType::Secure_Signed, ionet::PacketType::Chain_Info);
		});
	}

	TEST(TEST_CLASS, UnsupportedSecurityModeIsRejected) {
		// Arrange:
		ConnectionSettings settings;
		settings.OutgoingSecurityMode = ionet::ConnectionSecurityMode::None;
		settings.IncomingSecurityModes = ionet::ConnectionSecurityMode::Signed;

		// Act:
		RunSecurityModeTest(settings, true, [](const auto& state) {
			// Assert:
			EXPECT_EQ(PeerConnectCode::Verify_Error, state.ConnectCode);
			EXPECT_FALSE(!!state.pConnectedClientSocket);
		});
	}

	// endregion
}}
