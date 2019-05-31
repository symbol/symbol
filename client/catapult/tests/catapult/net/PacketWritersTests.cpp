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

#include "catapult/net/PacketWriters.h"
#include "catapult/crypto/KeyPair.h"
#include "catapult/ionet/BufferedPacketIo.h"
#include "catapult/ionet/Node.h"
#include "catapult/ionet/PacketSocket.h"
#include "catapult/net/VerifyPeer.h"
#include "catapult/thread/IoThreadPool.h"
#include "catapult/utils/TimeSpan.h"
#include "tests/test/core/AddressTestUtils.h"
#include "tests/test/core/KeyPairTestUtils.h"
#include "tests/test/core/ThreadPoolTestUtils.h"
#include "tests/test/net/NodeTestUtils.h"
#include "tests/test/net/SocketTestUtils.h"

namespace catapult { namespace net {

#define TEST_CLASS PacketWritersTests

	// region basic test utils

	namespace {
		const auto Default_Timeout = []() { return utils::TimeSpan::FromMinutes(1); }();

		auto CreateDefaultPacketWriters() {
			return CreatePacketWriters(test::CreateStartedIoThreadPool(), test::GenerateKeyPair(), ConnectionSettings());
		}

		void EmptyReadCallback(ionet::SocketOperationCode, const ionet::Packet*)
		{}

		void EmptyWriteCallback(ionet::SocketOperationCode)
		{}

		struct PacketWritersTestContext {
		public:
			PacketWritersTestContext(size_t numClientKeyPairs = 1)
					: ServerKeyPair(test::GenerateKeyPair())
					, pPool(test::CreateStartedIoThreadPool())
					, IoContext(pPool->ioContext())
					, pWriters(CreatePacketWriters(pPool, ServerKeyPair, ConnectionSettings())) {
				for (auto i = 0u; i < numClientKeyPairs; ++i)
					ClientKeyPairs.push_back(test::GenerateKeyPair());
			}

			~PacketWritersTestContext() {
				// if a test has already destroyed the writers, just wait for the pool threads to finish
				if (pWriters) {
					pWriters->shutdown();
					test::WaitForUnique(pWriters, "pWriters");
				}

				pPool->join();
			}

		public:
			crypto::KeyPair ServerKeyPair; // the server hosting the PacketWriters instance
			std::vector<crypto::KeyPair> ClientKeyPairs; // accepted clients forwarded to the server AND/OR connections initiated by server
			std::shared_ptr<thread::IoThreadPool> pPool;
			boost::asio::io_context& IoContext;
			std::shared_ptr<PacketWriters> pWriters;

		public:
			ionet::Node serverNode() const {
				return test::CreateLocalHostNode(ServerKeyPair.publicKey());
			}

			void waitForConnections(size_t numConnections) const {
				WAIT_FOR_VALUE_EXPR(numConnections, pWriters->numActiveConnections());
			}

			void waitForWriters(size_t numWriters) const {
				WAIT_FOR_VALUE_EXPR(numWriters, pWriters->numActiveWriters());
			}

			void waitForAvailableWriters(size_t numWriters) const {
				WAIT_FOR_VALUE_EXPR(numWriters, pWriters->numAvailableWriters());
			}
		};

		struct MultiConnectionState {
			std::vector<PeerConnectResult> Results;
			std::vector<std::shared_ptr<ionet::PacketSocket>> ServerSockets;
			std::vector<std::shared_ptr<ionet::PacketSocket>> ClientSockets;
		};

		class MultiConnectionStateGuard {
		public:
			explicit MultiConnectionStateGuard(PacketWriters& writers, MultiConnectionState& state)
					: m_writers(writers)
					, m_state(state)
			{}

			~MultiConnectionStateGuard() {
				m_writers.shutdown(); // release all server socket references held by the writers
				wait();
			}

		public:
			void wait() {
				// wait for any pending server socket operations to complete
				for (const auto& pSocket : m_state.ServerSockets)
					wait(pSocket);
			}

			void destroyServerSocketAt(size_t index) {
				auto& pSocket = m_state.ServerSockets[index];
				wait(pSocket);
				pSocket.reset();
			}

		private:
			void wait(const std::shared_ptr<ionet::PacketSocket>& pSocket) {
				if (!pSocket)
					return;

				test::WaitForUnique(pSocket, "pSocket");
			}

		private:
			PacketWriters& m_writers;
			MultiConnectionState& m_state;
		};

		MultiConnectionState SetupMultiConnectionTest(const PacketWritersTestContext& context, size_t numExpectedWriters = 0) {
			// Act: start multiple server and client verify operations
			MultiConnectionState state;
			test::TcpAcceptor acceptor(context.IoContext);

			auto numConnections = context.ClientKeyPairs.size();
			for (auto i = 0u; i < numConnections; ++i) {
				// - connect to nodes with different identities
				const auto& peerKeyPair = context.ClientKeyPairs[i];
				ionet::Node node(peerKeyPair.publicKey(), context.serverNode().endpoint(), ionet::NodeMetadata());

				std::atomic<size_t> numCallbacks(0);
				test::SpawnPacketServerWork(acceptor, [&](const auto& pSocket) {
					state.ServerSockets.push_back(pSocket);
					VerifyClient(pSocket, peerKeyPair, ionet::ConnectionSecurityMode::None, [&](auto, const auto&) {
						++numCallbacks;
					});
				});

				context.pWriters->connect(node, [&](const auto& connectResult) {
					state.Results.push_back(connectResult);
					++numCallbacks;
				});

				// - wait for both verifications to complete
				WAIT_FOR_VALUE(2u, numCallbacks);
			}

			context.waitForWriters(0 == numExpectedWriters ? numConnections : numExpectedWriters);
			return state;
		}

		MultiConnectionState SetupMultiConnectionAcceptTest(const PacketWritersTestContext& context, size_t numExpectedWriters = 0) {
			// Act: start multiple server and client verify operations
			MultiConnectionState state;
			test::TcpAcceptor acceptor(context.IoContext);

			auto numConnections = context.ClientKeyPairs.size();
			for (auto i = 0u; i < numConnections; ++i) {
				std::atomic<size_t> numCallbacks(0);
				test::SpawnPacketServerWork(acceptor, [&](const auto& pSocket) {
					state.ServerSockets.push_back(pSocket);
					context.pWriters->accept(pSocket, [&](auto acceptResult) {
						state.Results.push_back(acceptResult);
						++numCallbacks;
					});
				});

				bool isServerVerified = false;
				test::SpawnPacketClientWork(context.IoContext, [&, i](const auto& pSocket) {
					state.ClientSockets.push_back(pSocket);
					auto serverPeerInfo = VerifiedPeerInfo{ context.ServerKeyPair.publicKey(), ionet::ConnectionSecurityMode::None };
					VerifyServer(pSocket, serverPeerInfo, context.ClientKeyPairs[i], [&](auto result, const auto&) {
						isServerVerified = VerifyResult::Success == result;
						++numCallbacks;
					});
				});

				// - wait for both verifications to complete and make sure the client verified too
				WAIT_FOR_VALUE(2u, numCallbacks);
				EXPECT_TRUE(isServerVerified);
			}

			context.waitForWriters(0 == numExpectedWriters ? numConnections : numExpectedWriters);
			return state;
		}
	}

#define EXPECT_NUM_PENDING_WRITERS(EXPECTED_NUM_WRITERS, WRITERS) \
	EXPECT_EQ(EXPECTED_NUM_WRITERS, (WRITERS).numActiveConnections()); \
	EXPECT_EQ(0u, (WRITERS).numActiveWriters()); \
	EXPECT_EQ(0u, (WRITERS).identities().size()); \
	EXPECT_EQ(0u, (WRITERS).numAvailableWriters());

#define EXPECT_NUM_ACTIVE_WRITERS(EXPECTED_NUM_WRITERS, WRITERS) \
	EXPECT_EQ(EXPECTED_NUM_WRITERS, (WRITERS).numActiveConnections()); \
	EXPECT_EQ(EXPECTED_NUM_WRITERS, (WRITERS).numActiveWriters()); \
	EXPECT_EQ(EXPECTED_NUM_WRITERS, (WRITERS).identities().size()); \
	EXPECT_EQ(EXPECTED_NUM_WRITERS, (WRITERS).numAvailableWriters());

#define EXPECT_NUM_ACTIVE_AVAILABLE_WRITERS(EXPECTED_NUM_WRITERS, EXPECTED_NUM_AVAILABLE_WRITERS, WRITERS) \
	EXPECT_EQ(EXPECTED_NUM_WRITERS, (WRITERS).numActiveConnections()); \
	EXPECT_EQ(EXPECTED_NUM_WRITERS, (WRITERS).numActiveWriters()); \
	EXPECT_EQ(EXPECTED_NUM_WRITERS, (WRITERS).identities().size()); \
	EXPECT_EQ(EXPECTED_NUM_AVAILABLE_WRITERS, (WRITERS).numAvailableWriters());

	// endregion

	// region connect / accept failure

	TEST(TEST_CLASS, InitiallyNoConnectionsAreActive) {
		// Act:
		auto pWriters = CreateDefaultPacketWriters();

		// Assert:
		EXPECT_NUM_ACTIVE_WRITERS(0u, *pWriters);
	}

	TEST(TEST_CLASS, ConnectFailsOnConnectError) {
		// Arrange:
		PacketWritersTestContext context;
		std::atomic<size_t> numCallbacks(0);

		// Act: try to connect to a server that isn't running
		PeerConnectResult result;
		context.pWriters->connect(context.serverNode(), [&](const auto& connectResult) {
			result = connectResult;
			++numCallbacks;
		});
		WAIT_FOR_ONE(numCallbacks);

		// Assert:
		EXPECT_EQ(PeerConnectCode::Socket_Error, result.Code);
		EXPECT_NUM_ACTIVE_WRITERS(0u, *context.pWriters);
	}

	TEST(TEST_CLASS, AcceptFailsOnAcceptError) {
		// Arrange:
		PacketWritersTestContext context;

		// Act: on an accept error, the server will pass nullptr
		PeerConnectResult result;
		context.pWriters->accept(nullptr, [&](auto acceptResult) { result = acceptResult; });

		// Assert:
		EXPECT_EQ(PeerConnectCode::Socket_Error, result.Code);
		EXPECT_NUM_ACTIVE_WRITERS(0u, *context.pWriters);
	}

	TEST(TEST_CLASS, ConnectFailsOnVerifyError) {
		// Arrange:
		PacketWritersTestContext context;
		std::atomic<size_t> numCallbacks(0);

		// Act: start a server and client verify operation
		PeerConnectResult result;
		test::SpawnPacketServerWork(context.IoContext, [&](const auto& pSocket) {
			// - trigger a verify error by closing the socket without responding
			pSocket->close();
			++numCallbacks;
		});

		context.pWriters->connect(context.serverNode(), [&](auto connectResult) {
			result = connectResult;
			++numCallbacks;
		});

		// - wait for both callbacks to complete and the connection to close
		WAIT_FOR_VALUE(2u, numCallbacks);
		context.waitForConnections(0);

		// Assert: the verification should have failed and all connections should have been destroyed
		EXPECT_EQ(PeerConnectCode::Verify_Error, result.Code);
		EXPECT_NUM_ACTIVE_WRITERS(0u, *context.pWriters);
	}

	TEST(TEST_CLASS, AcceptFailsOnVerifyError) {
		// Arrange:
		PacketWritersTestContext context;
		std::atomic<size_t> numCallbacks(0);

		// Act: start a server and client verify operation
		PeerConnectResult result;
		test::SpawnPacketServerWork(context.IoContext, [&](const auto& pSocket) {
			context.pWriters->accept(pSocket, [&](auto acceptResult) {
				result = acceptResult;
				++numCallbacks;
			});
		});

		test::SpawnPacketClientWork(context.IoContext, [&](const auto& pSocket) {
			// - trigger a verify error by closing the socket without responding
			pSocket->close();
			++numCallbacks;
		});

		// - wait for both callbacks to complete and the connection to close
		WAIT_FOR_VALUE(2u, numCallbacks);
		context.waitForConnections(0);

		// Assert: the verification should have failed and all connections should have been destroyed
		EXPECT_EQ(PeerConnectCode::Verify_Error, result.Code);
		EXPECT_NUM_ACTIVE_WRITERS(0u, *context.pWriters)
	}

	// endregion

	// region connected / accepted writer

	namespace {
		using ResultServerClientHandler = consumer<const PeerConnectResult&, ionet::PacketSocket&>;

		void RunConnectedSocketTest(const PacketWritersTestContext& context, const ResultServerClientHandler& handler) {
			// Act: establish a single connection
			auto state = SetupMultiConnectionTest(context);

			// Assert: call the handler
			handler(state.Results.back(), *state.ServerSockets.back());
		}

		void RunAcceptedSocketTest(const PacketWritersTestContext& context, const ResultServerClientHandler& handler) {
			// Act: establish a single connection
			auto state = SetupMultiConnectionAcceptTest(context);

			// Assert: call the handler
			handler(state.Results.back(), *state.ServerSockets.back());
		}
	}

	TEST(TEST_CLASS, ConnectSucceedsOnVerifySuccess) {
		// Act:
		PacketWritersTestContext context;
		RunConnectedSocketTest(context, [&](const auto& connectResult, const auto&) {
			// Assert: the verification should have succeeded and the connection should be active
			EXPECT_EQ(PeerConnectCode::Accepted, connectResult.Code);
			EXPECT_EQ(context.ClientKeyPairs[0].publicKey(), connectResult.IdentityKey);
			EXPECT_NUM_ACTIVE_WRITERS(1u, *context.pWriters);
			EXPECT_EQ(test::ToKeySet(context.ClientKeyPairs), context.pWriters->identities());
		});
	}

	TEST(TEST_CLASS, AcceptSucceedsOnVerifySuccess) {
		// Act:
		PacketWritersTestContext context;
		RunAcceptedSocketTest(context, [&](const auto& connectResult, const auto&) {
			// Assert: the verification should have succeeded and the connection should be active
			EXPECT_EQ(PeerConnectCode::Accepted, connectResult.Code);
			EXPECT_EQ(context.ClientKeyPairs[0].publicKey(), connectResult.IdentityKey);
			EXPECT_NUM_ACTIVE_WRITERS(1u, *context.pWriters);
			EXPECT_EQ(test::ToKeySet(context.ClientKeyPairs), context.pWriters->identities());
		});
	}

	TEST(TEST_CLASS, ShutdownClosesConnectedSocket) {
		// Act:
		PacketWritersTestContext context;
		RunConnectedSocketTest(context, [&](auto, const auto&) {
			// Act: shutdown the writers
			context.pWriters->shutdown();

			// Assert: socket was disconnected
			// - the serverSocket parameter refers to the other side of the connection, so it is still open
			EXPECT_NUM_ACTIVE_WRITERS(0u, *context.pWriters);
		});
	}

	TEST(TEST_CLASS, ShutdownClosesAcceptedSocket) {
		// Act:
		PacketWritersTestContext context;
		RunAcceptedSocketTest(context, [&](auto, auto& serverSocket) {
			// Act: shutdown the writers
			context.pWriters->shutdown();

			// Assert: the server socket was closed
			EXPECT_FALSE(test::IsSocketOpen(serverSocket));
			EXPECT_NUM_ACTIVE_WRITERS(0u, *context.pWriters);
		});
	}

	TEST(TEST_CLASS, CanManageMultipleConnections) {
		// Act: establish multiple connections
		constexpr auto Num_Connections = 5u;
		PacketWritersTestContext context(Num_Connections);
		auto state = SetupMultiConnectionTest(context);

		// Assert: all connections are active
		auto i = 0u;
		EXPECT_EQ(Num_Connections, state.Results.size());
		for (const auto& result : state.Results) {
			EXPECT_EQ(PeerConnectCode::Accepted, result.Code);
			EXPECT_EQ(context.ClientKeyPairs[i].publicKey(), result.IdentityKey);
			++i;
		}

		EXPECT_NUM_ACTIVE_WRITERS(Num_Connections, *context.pWriters);
		EXPECT_EQ(test::ToKeySet(context.ClientKeyPairs), context.pWriters->identities());
	}

	// the connected tests equivalent to OnlyOneConnectionIsAllowedPerAcceptedIdentity
	// are CannotConnectTo(Already)ConnectedPeer

	TEST(TEST_CLASS, OnlyOneConnectionIsAllowedPerAcceptedIdentity) {
		// Act: establish multiple connections with the same identity
		constexpr auto Num_Connections = 5u;
		PacketWritersTestContext context(Num_Connections);
		for (auto& keyPair : context.ClientKeyPairs)
			keyPair = test::CopyKeyPair(context.ClientKeyPairs[0]);

		auto state = SetupMultiConnectionAcceptTest(context, 1);

		// Assert: all connections succeeded but only a single one is active
		EXPECT_EQ(Num_Connections, state.Results.size());
		EXPECT_EQ(PeerConnectCode::Accepted, state.Results[0].Code);
		for (auto i = 1u; i < state.Results.size(); ++i)
			EXPECT_EQ(PeerConnectCode::Already_Connected, state.Results[i].Code) << "result at " << i;

		EXPECT_EQ(Num_Connections, context.pWriters->numActiveConnections());
		EXPECT_EQ(1u, context.pWriters->numActiveWriters());
		EXPECT_EQ(test::ToKeySet(context.ClientKeyPairs), context.pWriters->identities());

		// Sanity: closing the corresponding server sockets removes the pending connections
		state.ServerSockets.clear();
		context.waitForConnections(1);
		EXPECT_NUM_ACTIVE_WRITERS(1u, *context.pWriters);
	}

	TEST(TEST_CLASS, AcceptIsRejectedWhenIdentityIsAlreadyConnected) {
		// Arrange: establish a single connection (via connect)
		PacketWritersTestContext context(1);
		auto state1 = SetupMultiConnectionTest(context);

		// Sanity: a single connection is active
		EXPECT_NUM_ACTIVE_WRITERS(1u, *context.pWriters);

		// Act: attempt to connect via accept
		auto state2 = SetupMultiConnectionAcceptTest(context);

		// Assert: only a single writer remains
		EXPECT_EQ(PeerConnectCode::Accepted, state1.Results[0].Code);
		EXPECT_EQ(PeerConnectCode::Already_Connected, state2.Results[0].Code);

		EXPECT_EQ(1u, context.pWriters->numActiveWriters());
		EXPECT_EQ(test::ToKeySet(context.ClientKeyPairs), context.pWriters->identities());

		// Sanity: closing the corresponding server socket removes the pending connection too
		state2.ServerSockets[0].reset();
		context.waitForConnections(1);
		EXPECT_NUM_ACTIVE_WRITERS(1u, *context.pWriters);
		EXPECT_EQ(test::ToKeySet(context.ClientKeyPairs), context.pWriters->identities());
	}

	TEST(TEST_CLASS, ConnectIsRejectedWhenIdentityIsAlreadyAccepted) {
		// Arrange: establish a single connection (via accept)
		PacketWritersTestContext context(1);
		auto state1 = SetupMultiConnectionAcceptTest(context);

		// Sanity: a single connection is active
		EXPECT_NUM_ACTIVE_WRITERS(1u, *context.pWriters);

		// Act: try to connect to the same node again
		auto state2 = SetupMultiConnectionTest(context);

		// Assert: only a single connection succeded
		EXPECT_EQ(PeerConnectCode::Accepted, state1.Results[0].Code);
		EXPECT_EQ(PeerConnectCode::Already_Connected, state2.Results[0].Code);

		EXPECT_NUM_ACTIVE_WRITERS(1u, *context.pWriters);
		EXPECT_EQ(test::ToKeySet(context.ClientKeyPairs), context.pWriters->identities());
	}

	// endregion

	// region connecting / accepting writer

	namespace {
		void RunConnectingSocketTest(const PacketWritersTestContext& context, const ResultServerClientHandler& handler) {
			std::atomic<size_t> numCallbacks(0);

			// Act: start a verify operation that the server does not respond to
			std::shared_ptr<ionet::PacketSocket> pServerSocket;
			test::SpawnPacketServerWork(context.IoContext, [&](const auto& pSocket) {
				pServerSocket = pSocket;
				++numCallbacks;
			});

			// - (use a result shared_ptr so that the connect callback is valid even after this function returns)
			auto pResult = std::make_shared<PeerConnectResult>(static_cast<PeerConnectCode>(-1));
			context.pWriters->connect(context.serverNode(), [&, pResult](const auto& connectResult) {
				// note that this is not expected to get called until shutdown because the client doesn't read
				// or write any data
				*pResult = connectResult;
			});

			// - wait for the initial work to complete
			WAIT_FOR_ONE(numCallbacks);
			context.waitForConnections(1);

			// Assert: the client connect handler was never called
			EXPECT_EQ(static_cast<PeerConnectCode>(-1), pResult->Code);

			// - call the test handler
			handler(*pResult, *pServerSocket);
		}

		void RunConnectingAcceptSocketTest(const PacketWritersTestContext& context, const ResultServerClientHandler& handler) {
			std::atomic<size_t> numCallbacks(0);

			// Act: start a verify operation that the client does not respond to
			//      (use a result shared_ptr so that the accept callback is valid even after this function returns)
			auto pResult = std::make_shared<PeerConnectResult>(static_cast<PeerConnectCode>(-1));
			std::shared_ptr<ionet::PacketSocket> pServerSocket;
			test::SpawnPacketServerWork(context.IoContext, [&, pResult](const auto& pSocket) {
				pServerSocket = pSocket;
				context.pWriters->accept(pSocket, [&, pResult](const auto& acceptResult) {
					// note that this is not expected to get called until shutdown because the client doesn't read
					// or write any data
					*pResult = acceptResult;
				});
				++numCallbacks;
			});

			std::shared_ptr<ionet::PacketSocket> pClientSocket;
			test::SpawnPacketClientWork(context.IoContext, [&](const auto& pSocket) {
				pClientSocket = pSocket;
				++numCallbacks;
			});

			// - wait for the initial work to complete and the connection to become active
			WAIT_FOR_VALUE(2u, numCallbacks);
			context.waitForConnections(1);

			// Assert: the server accept handler was never called
			EXPECT_EQ(static_cast<PeerConnectCode>(-1), pResult->Code);

			// - call the test handler
			handler(*pResult, *pServerSocket);
		}
	}

	TEST(TEST_CLASS, ConnectingConnectionIsIncludedInNumActiveConnections) {
		test::RunNonDeterministicTest("numActiveConnections includes connecting connections", []() {
			// Arrange: start a verify operation that the server does not respond to
			PacketWritersTestContext context;
			test::SpawnPacketServerWork(context.IoContext, [](const auto&) {});

			// Act: start a connect
			context.pWriters->connect(context.serverNode(), [](auto) {
				// note that this is not expected to get called until shutdown because the client doesn't read
				// or write any data
			});

			// Assert: the pending writers counter was incremented
			auto numActiveConnections = context.pWriters->numActiveConnections();
			EXPECT_GT(2u, numActiveConnections);
			return 1u == numActiveConnections;
		});
	}

	TEST(TEST_CLASS, VerifyingConnectionIsIncludedInNumActiveConnections) {
		// Act:
		PacketWritersTestContext context;
		RunConnectingSocketTest(context, [&](auto, const auto&) {
			// Assert: the verifying connection is active
			EXPECT_NUM_PENDING_WRITERS(1u, *context.pWriters);
		});
	}

	TEST(TEST_CLASS, VerifyingAcceptConnectionIsIncludedInNumActiveConnections) {
		// Act:
		PacketWritersTestContext context;
		RunConnectingAcceptSocketTest(context, [&](auto, const auto&) {
			// Assert: the verifying connection is active
			EXPECT_NUM_PENDING_WRITERS(1u, *context.pWriters);
		});
	}

	TEST(TEST_CLASS, ShutdownClosesVerifyingSocket) {
		// Act:
		PacketWritersTestContext context;
		RunConnectingSocketTest(context, [&](auto, const auto&) {
			// Act: shutdown the writers
			context.pWriters->shutdown();

			// Assert: socket was disconnected
			// - the serverSocket parameter refers to the other side of the connection, so it is still open
			EXPECT_NUM_ACTIVE_WRITERS(0u, *context.pWriters);
		});
	}

	TEST(TEST_CLASS, ShutdownClosesVerifyingAcceptedSocket) {
		// Act:
		PacketWritersTestContext context;
		RunConnectingAcceptSocketTest(context, [&](auto, auto& serverSocket) {
			// Act: shutdown the writers
			context.pWriters->shutdown();

			// Assert: the server socket was closed
			EXPECT_FALSE(test::IsSocketOpen(serverSocket));
			EXPECT_NUM_ACTIVE_WRITERS(0u, *context.pWriters);
		});
	}

	// endregion

	// region broadcast

	namespace {
		constexpr uint8_t Sentinel_Index = 50;

		using CounterPointer = std::shared_ptr<std::atomic<size_t>>;

		auto CreateCounterPointer() {
			return std::make_shared<std::atomic<size_t>>(0);
		}

		auto HandleSocketReadInSendTests(const CounterPointer& pCounter, const ionet::ByteBuffer& buffer, bool expectSuccess = true) {
			return [pCounter, buffer, expectSuccess](auto code, const auto* pPacket) {
				CATAPULT_LOG(debug) << "read from socket returned " << code;

				if (expectSuccess) {
					EXPECT_EQ(ionet::SocketOperationCode::Success, code);

					auto pPacketData = reinterpret_cast<const uint8_t*>(pPacket);
					CATAPULT_LOG(debug)
							<< "read sentinel value " << static_cast<int>(pPacketData[Sentinel_Index])
							<< " (expected " << static_cast<int>(buffer[Sentinel_Index]) << ")";

					EXPECT_EQ(buffer[Sentinel_Index], pPacketData[Sentinel_Index]);
					EXPECT_EQ(test::ToHexString(buffer), test::ToHexString(pPacketData, pPacket->Size));
				} else {
					EXPECT_EQ(ionet::SocketOperationCode::Closed, code);
					EXPECT_FALSE(!!pPacket);
				}

				++*pCounter;
			};
		}
	}

	TEST(TEST_CLASS, CanBroadcastPacketToAllPeers) {
		// Arrange: establish multiple connections
		constexpr auto Num_Connections = 5u;
		PacketWritersTestContext context(Num_Connections);
		auto state = SetupMultiConnectionTest(context);
		MultiConnectionStateGuard stateGuard(*context.pWriters, state);

		// Act: broadcast a random packet
		auto buffer = test::GenerateRandomPacketBuffer(95);
		context.pWriters->broadcast(test::BufferToPacketPayload(buffer));

		// Assert: the packet was sent to all connected sockets
		auto pNumReads = CreateCounterPointer();
		for (const auto& pSocket : state.ServerSockets)
			pSocket->read(HandleSocketReadInSendTests(pNumReads, buffer));

		WAIT_FOR_VALUE(Num_Connections, *pNumReads);

		// - all connections are still open
		EXPECT_NUM_ACTIVE_WRITERS(Num_Connections, *context.pWriters);
	}

	TEST(TEST_CLASS, BroadcastClosesPeersThatFail) {
		// Arrange: establish multiple connections
		constexpr auto Num_Connections = 5u;
		PacketWritersTestContext context(Num_Connections);
		auto state = SetupMultiConnectionAcceptTest(context);
		MultiConnectionStateGuard stateGuard(*context.pWriters, state);

		// - close two of the server (accepted) sockets
		state.ServerSockets[2]->close();
		state.ServerSockets[3]->close();

		// Act: broadcast a random packet
		auto buffer = test::GenerateRandomPacketBuffer(95);
		context.pWriters->broadcast(test::BufferToPacketPayload(buffer));

		// Assert: the packet was sent to all connected (client) sockets
		size_t i = 0;
		auto pNumReads = CreateCounterPointer();
		for (const auto& pSocket : state.ClientSockets) {
			pSocket->read(HandleSocketReadInSendTests(pNumReads, buffer, 2 != i && 3 != i));
			++i;
		}

		WAIT_FOR_VALUE(Num_Connections, *pNumReads);

		// - release the shared pointers in state to the closed server sockets (so that the weak pointer
		//   cannot be locked)
		stateGuard.destroyServerSocketAt(2);
		stateGuard.destroyServerSocketAt(3);

		// Assert: the closed server sockets have been removed from the active connections
		EXPECT_NUM_ACTIVE_WRITERS(3u, *context.pWriters);
	}

	// endregion

	// region pickOne

	TEST(TEST_CLASS, PickOneEvenlyRotatesPeers) {
		// Arrange: establish multiple connections
		constexpr auto Num_Packets_Per_Socket = 4u;
		constexpr auto Num_Connections = 5u;
		PacketWritersTestContext context(Num_Connections);
		auto state = SetupMultiConnectionTest(context);
		MultiConnectionStateGuard stateGuard(*context.pWriters, state);

		// Act: send multiple packets to each socket
		for (auto i = 0u; i < Num_Packets_Per_Socket; ++i) {
			// - send a different packet to each socket
			auto buffer = test::GenerateRandomPacketBuffer(95);
			for (auto j = 0u; j < Num_Connections; ++j) {
				auto sentinelValue = 0x80 | (j + i * Num_Connections);
				buffer[Sentinel_Index] = static_cast<uint8_t>(sentinelValue);
				CATAPULT_LOG(debug) << "sending packet " << j << " (sentinel " << sentinelValue << ")";
				context.pWriters->pickOne(Default_Timeout).io()->write(test::BufferToPacketPayload(buffer), EmptyWriteCallback);
			}

			// Assert: the packets were sent to all connected sockets
			size_t socketId = 0u;
			auto pNumReads = CreateCounterPointer();
			for (const auto& pSocket : state.ServerSockets) {
				auto expectedBuffer = buffer;
				expectedBuffer[Sentinel_Index] = static_cast<uint8_t>(0x80 | (socketId + i * Num_Connections));
				pSocket->read(HandleSocketReadInSendTests(pNumReads, expectedBuffer));
				++socketId;
			}

			// - wait for all reads and for all connections to be returned
			WAIT_FOR_VALUE(Num_Connections, *pNumReads);
			context.waitForAvailableWriters(Num_Connections);
		}

		// - all connections are still open
		EXPECT_NUM_ACTIVE_WRITERS(Num_Connections, *context.pWriters);
	}

	TEST(TEST_CLASS, PickOneReturnsNullWhenThereAreNoPeers) {
		// Arrange: create a context with no connections
		PacketWritersTestContext context;

		// Act: get an io wrapper from the writers
		auto pIo = context.pWriters->pickOne(Default_Timeout).io();

		// Assert: the io wrapper is null and there are still no connections
		EXPECT_FALSE(!!pIo);
		EXPECT_NUM_ACTIVE_WRITERS(0u, *context.pWriters);
	}

	namespace {
		void AssertOperationClosesConnectedSocket(const consumer<ionet::PacketSocket&, PacketWriters&>& operation) {
			// Act: create a connection
			PacketWritersTestContext context;
			RunConnectedSocketTest(context, [&](auto, auto& socket) {
				// Sanity: the connection is active
				EXPECT_NUM_ACTIVE_WRITERS(1u, *context.pWriters);

				// Act: trigger the operation that should close the socket and wait for the connections to drop
				operation(socket, *context.pWriters);
				context.waitForConnections(0);

				// Assert: the connection is closed
				EXPECT_NUM_ACTIVE_WRITERS(0u, *context.pWriters);
			});
		}

		void AssertNonDeterministicOperationClosesConnectedSocket(
				const predicate<ionet::PacketSocket&, PacketWriters&, size_t>& operation) {
			test::RunNonDeterministicTest("closes socket test", [&operation](auto i) {
				// Act: create a connection
				bool isResultDeterministic = true;
				PacketWritersTestContext context;
				RunConnectedSocketTest(context, [&](auto, auto& socket) {
					// Sanity: the connection is active
					EXPECT_NUM_ACTIVE_WRITERS(1u, *context.pWriters);

					// Act: trigger the (non-deterministic) operation that should close the socket
					if (!operation(socket, *context.pWriters, i)) {
						isResultDeterministic = false;
						return;
					}

					// - wait for the connections to drop
					context.waitForConnections(0);

					// Assert: the connection is closed
					EXPECT_NUM_ACTIVE_WRITERS(0u, *context.pWriters);
				});

				return isResultDeterministic;
			});
		}
	}

	TEST(TEST_CLASS, PickOneReadErrorClosesPeer) {
		AssertOperationClosesConnectedSocket([](auto& socket, auto& writers) {
			// Arrange:
			std::atomic<size_t> numCallbacks(0);
			auto readCode = ionet::SocketOperationCode::Success;

			// Act: close the server socket and attempt to read from it (the read should fail)
			socket.close();
			writers.pickOne(Default_Timeout).io()->read([&](auto code, const auto*) { readCode = code; ++numCallbacks; });
			WAIT_FOR_ONE(numCallbacks);

			// Assert: the connection is closed
			EXPECT_EQ(ionet::SocketOperationCode::Closed, readCode);
		});
	}

	TEST(TEST_CLASS, PickOneTimeoutClosesPeerAndPreventsSubsequentReads) {
		// Assert: non-deterministic due to timeouts
		AssertNonDeterministicOperationClosesConnectedSocket([](auto& socket, auto& writers, size_t i) {
			// Arrange:
			std::atomic<size_t> numCallbacks(0);
			auto readCode = ionet::SocketOperationCode::Success;

			// - write some dummy data to the socket
			socket.write(test::BufferToPacketPayload(test::GenerateRandomPacketBuffer(95)), EmptyWriteCallback);

			// Act: get an io wrapper, wait for it to timeout, then attempt to read from the io wrapper
			auto pIo = writers.pickOne(utils::TimeSpan::FromMilliseconds(1)).io();
			test::Sleep(static_cast<long>(5 * i));
			pIo->read([&](auto code, const auto*) { readCode = code; ++numCallbacks; });
			WAIT_FOR_ONE(numCallbacks);

			if (ionet::SocketOperationCode::Success == readCode)
				return false;

			// Assert: the connection is closed
			EXPECT_EQ(ionet::SocketOperationCode::Read_Error, readCode);
			return true;
		});
	}

	TEST(TEST_CLASS, PickOneWriteErrorClosesPeer) {
		AssertOperationClosesConnectedSocket([](const auto&, auto& writers) {
			// Arrange:
			std::atomic<size_t> numCallbacks(0);
			auto writeCode = ionet::SocketOperationCode::Success;
			auto pPacket = test::BufferToPacketPayload(test::GenerateRandomPacketBuffer(95));

			// Act: get an io wrapper, shutdown the writers, then attempt to write to the io wrapper
			auto pIo = writers.pickOne(Default_Timeout).io();
			writers.shutdown();
			pIo->write(std::move(pPacket), [&](auto code) { writeCode = code; ++numCallbacks; });
			WAIT_FOR_ONE(numCallbacks);

			// Assert: the connection is closed
			EXPECT_EQ(ionet::SocketOperationCode::Write_Error, writeCode);
		});
	}

	TEST(TEST_CLASS, PickOneTimeoutClosesPeerAndPreventsSubsequentWrites) {
		// Assert: non-deterministic due to timeouts
		AssertNonDeterministicOperationClosesConnectedSocket([](const auto&, auto& writers, auto i) {
			// Arrange:
			std::atomic<size_t> numCallbacks(0);
			auto writeCode = ionet::SocketOperationCode::Success;
			auto pPacket = test::BufferToPacketPayload(test::GenerateRandomPacketBuffer(95));

			// Act: get an io wrapper, wait for it to timeout, then attempt to write to the io wrapper
			auto pIo = writers.pickOne(utils::TimeSpan::FromMilliseconds(1)).io();
			test::Sleep(static_cast<long>(5 * i));
			pIo->write(std::move(pPacket), [&](auto code) { writeCode = code; ++numCallbacks; });
			WAIT_FOR_ONE(numCallbacks);

			if (ionet::SocketOperationCode::Success == writeCode)
				return false;

			// Assert: the connection is closed
			EXPECT_EQ(ionet::SocketOperationCode::Write_Error, writeCode);
			return true;
		});
	}

	namespace {
		void RunTestWithClosedPacketIo(const consumer<ionet::PacketIo&>& operation) {
			AssertOperationClosesConnectedSocket([operation](auto& socket, auto& writers) {
				// Arrange: close the server socket and attempt to read from it (the read should fail)
				socket.close();
				auto pIo = writers.pickOne(Default_Timeout).io();
				pIo->read(EmptyReadCallback);
				WAIT_FOR_ZERO_EXPR(writers.numActiveWriters());

				// Sanity: the writer was removed from the writers because of the read error triggered above
				EXPECT_EQ(0u, writers.numActiveWriters());

				// Act: trigger the operation
				operation(*pIo);
			});
		}
	}

	TEST(TEST_CLASS, PickOneReadAfterCloseTriggersError) {
		RunTestWithClosedPacketIo([](auto& io) {
			// Arrange:
			std::atomic<size_t> numCallbacks(0);
			auto readCode = ionet::SocketOperationCode::Success;

			// Act:
			io.read([&](auto code, const auto*) { readCode = code; ++numCallbacks; });
			WAIT_FOR_ONE(numCallbacks);

			// Assert: the connection is closed
			EXPECT_EQ(ionet::SocketOperationCode::Read_Error, readCode);
		});
	}

	TEST(TEST_CLASS, PickOneWriteAfterCloseTriggersError) {
		RunTestWithClosedPacketIo([](auto& io) {
			// Arrange:
			std::atomic<size_t> numCallbacks(0);
			auto pPacket = test::BufferToPacketPayload(test::GenerateRandomPacketBuffer(95));
			auto writeCode = ionet::SocketOperationCode::Success;

			// Act:
			io.write(std::move(pPacket), [&](auto code) { writeCode = code; ++numCallbacks; });
			WAIT_FOR_ONE(numCallbacks);

			// Assert: the connection is closed
			EXPECT_EQ(ionet::SocketOperationCode::Write_Error, writeCode);
		});
	}

	namespace {
		using IoOperationCallback = consumer<ionet::SocketOperationCode>;
		void AssertOperationCanCompleteWithDestroyedWriters(
				const consumer<ionet::PacketIo&, const IoOperationCallback&>& operation,
				const consumer<ionet::SocketOperationCode>& assertCallbackCode) {
			// Act: create a connection
			PacketWritersTestContext context;
			RunConnectedSocketTest(context, [&](auto, const auto&) {
				// Sanity: the connection is active
				EXPECT_NUM_ACTIVE_WRITERS(1u, *context.pWriters);

				// Arrange:
				std::atomic<size_t> numCallbacks(0);
				auto callbackCode = ionet::SocketOperationCode::Success;

				// - start an operation and immediately destroy the io and shutdown and destroy the writers
				CATAPULT_LOG(debug) << "starting operation";
				auto pIo = context.pWriters->pickOne(Default_Timeout).io();
				operation(*pIo, [&](auto code) {
					CATAPULT_LOG(debug) << "operation completed";
					callbackCode = code;
					++numCallbacks;
				});
				pIo.reset();
				context.pWriters->shutdown();
				context.pWriters.reset();
				CATAPULT_LOG(debug) << "writers destroyed";
				WAIT_FOR_ONE(numCallbacks);

				// Assert: the operation result
				assertCallbackCode(callbackCode);
			});
		}
	}

	TEST(TEST_CLASS, PickOneReadCanCompleteWithDestroyedWriters) {
		// Assert:
		AssertOperationCanCompleteWithDestroyedWriters(
				[](auto& io, const auto& callback) {
					io.read([callback](auto code, const auto*) {
						callback(code);
					});
				},
				test::AssertSocketClosedDuringRead);
	}

	TEST(TEST_CLASS, PickOneWriteCanCompleteWithDestroyedWriters) {
		// Assert:
		AssertOperationCanCompleteWithDestroyedWriters(
				[](auto& io, const auto& callback) {
					// use a large packet to ensure the operation gets cancelled
					auto pPacket = test::BufferToPacketPayload(test::GenerateRandomPacketBuffer(5 * 1024 * 1024));
					io.write(std::move(pPacket), [callback](auto code) {
						callback(code);
					});
				},
				test::AssertSocketClosedDuringWrite);
	}

	// endregion

	// region data integrity (broadcast + pickOne)

	namespace {
		using SendFunction = consumer<PacketWriters&, const ionet::PacketPayload&>;
		void RunVerifyWrittenDataTest(size_t numConnections, const SendFunction& send) {
			// Arrange: connect to the specified number of nodes
			std::atomic<size_t> numPacketsRead(0);
			PacketWritersTestContext context(numConnections);
			auto state = SetupMultiConnectionAcceptTest(context);

			// Act: send a packet using the supplied function
			auto broadcastBuffer = test::GenerateRandomPacketBuffer(50);
			send(*context.pWriters, test::BufferToPacketPayload(broadcastBuffer));

			// - read from all the client sockets
			size_t socketId = 0;
			std::vector<ionet::ByteBuffer> receiveBuffers(numConnections);
			for (const auto& pSocket : state.ClientSockets) {
				pSocket->read([&, socketId](auto, const auto* pPacket) {
					auto receiveBuffer = test::CopyPacketToBuffer(*pPacket);
					receiveBuffers[socketId] = receiveBuffer;
					++numPacketsRead;
				});
				++socketId;
			}

			// - wait for all packets to be read
			WAIT_FOR_VALUE(numConnections, numPacketsRead);

			// Assert: the handler was called once for each socket with the same buffer
			auto broadcastBufferHex = test::ToHexString(broadcastBuffer);
			for (auto i = 0u; i < numConnections; ++i) {
				auto receiveBufferHex = test::ToHexString(receiveBuffers[i]);
				EXPECT_EQ(broadcastBufferHex, receiveBufferHex) << "tagged packet " << i;
			}
		}

		void RunVerifyReadDataTest(const consumer<PacketWriters&, const consumer<const ionet::Packet&>&>& receive) {
			// Arrange: connect to a single node
			std::atomic<size_t> numPacketsRead(0);
			PacketWritersTestContext context;
			auto state = SetupMultiConnectionAcceptTest(context);

			// - write a packet to the socket
			auto sendBuffer = test::GenerateRandomPacketBuffer(50);
			state.ClientSockets[0]->write(test::BufferToPacketPayload(sendBuffer), EmptyWriteCallback);

			// Act: read the packet using the supplied function
			ionet::ByteBuffer receiveBuffer;
			receive(*context.pWriters, [&](const auto& packet) {
				receiveBuffer = test::CopyPacketToBuffer(packet);
				++numPacketsRead;
			});

			// - wait for the packet to be read
			WAIT_FOR_ONE(numPacketsRead);

			// Assert: the expected packet was read
			EXPECT_EQ(test::ToHexString(sendBuffer), test::ToHexString(receiveBuffer));
		}
	}

	TEST(TEST_CLASS, BroadcastDataIsWrittenCorrectlyWhenSingleSocketIsActive) {
		// Assert:
		RunVerifyWrittenDataTest(1, [](auto& writers, const auto& payload) { writers.broadcast(payload); });
	}

	TEST(TEST_CLASS, BroadcastDataIsWrittenCorrectlyWhenMultipleSocketsAreActive) {
		// Assert:
		RunVerifyWrittenDataTest(4, [](auto& writers, const auto& payload) { writers.broadcast(payload); });
	}

	TEST(TEST_CLASS, PickOneWrapperWritesDataCorrectly) {
		// Assert:
		RunVerifyWrittenDataTest(1, [](auto& writers, const auto& payload) {
			writers.pickOne(Default_Timeout).io()->write(payload, EmptyWriteCallback);
		});
	}

	TEST(TEST_CLASS, PickOneWrapperWritesDataCorrectlyWithDestroyedWrapper) {
		// Assert:
		RunVerifyWrittenDataTest(1, [](auto& writers, const auto& payload) {
			auto pIo = writers.pickOne(Default_Timeout).io();
			pIo->write(payload, [](auto) { CATAPULT_LOG(debug) << "write completed"; });
			pIo.reset();
			CATAPULT_LOG(debug) << "destroyed wrapper";
		});
	}

	TEST(TEST_CLASS, PickOneWrapperReadsDataCorrectly) {
		// Assert:
		RunVerifyReadDataTest([](auto& writers, const auto& callback) {
			writers.pickOne(Default_Timeout).io()->read([callback](auto, const auto* pPacket) { callback(*pPacket); });
		});
	}

	TEST(TEST_CLASS, PickOneWrapperReadsDataCorrectlyWithDestroyedWrapper) {
		// Assert:
		RunVerifyReadDataTest([](auto& writers, const auto& callback) {
			auto pIo = writers.pickOne(Default_Timeout).io();
			pIo->read([callback](auto, const auto* pPacket) {
				CATAPULT_LOG(debug) << "read completed";
				callback(*pPacket);
			});
			pIo.reset();
			CATAPULT_LOG(debug) << "destroyed wrapper";
		});
	}

	// endregion

	// region checkout (broadcast + pickOne)

	TEST(TEST_CLASS, PickOneDecrementsNumAvailableWriters) {
		// Arrange: connect to 3 nodes
		PacketWritersTestContext context(3);
		auto& writers = *context.pWriters;
		auto state = SetupMultiConnectionAcceptTest(context);

		// Act: pick 2 / 3 sockets
		auto pIo1 = writers.pickOne(Default_Timeout).io();
		auto pIo2 = writers.pickOne(Default_Timeout).io();

		// Assert: 2 sockets are checked out, so only 1 is available
		EXPECT_NUM_ACTIVE_AVAILABLE_WRITERS(3u, 1u, writers);
	}

	TEST(TEST_CLASS, PickOneWriterIncrementsNumAvailableWritersWhenDestroyed) {
		// Arrange: connect to 3 nodes
		PacketWritersTestContext context(3);
		auto& writers = *context.pWriters;
		auto state = SetupMultiConnectionAcceptTest(context);

		// Act: pick 2 / 3 sockets and then destroy one
		auto pIo1 = writers.pickOne(Default_Timeout).io();
		auto pIo2 = writers.pickOne(Default_Timeout).io();
		pIo1.reset();

		context.waitForAvailableWriters(2);

		// Assert: 1 socket is checked out, so only 2 are available
		EXPECT_NUM_ACTIVE_AVAILABLE_WRITERS(3u, 2u, writers);
	}

	TEST(TEST_CLASS, PickOneChecksOutSockets) {
		// Arrange: connect to 2 nodes
		PacketWritersTestContext context(2);
		auto& writers = *context.pWriters;
		auto state = SetupMultiConnectionAcceptTest(context);
		auto sendBuffers = test::GenerateRandomPacketBuffers({ 51, 52 });

		// Act: pick 3 / 2 sockets
		auto pIo1 = writers.pickOne(Default_Timeout).io();
		auto pIo2 = writers.pickOne(Default_Timeout).io();
		auto pIo3 = writers.pickOne(Default_Timeout).io();

		// Assert: only available sockets were returned
		ASSERT_TRUE(!!pIo1);
		ASSERT_TRUE(!!pIo2);
		EXPECT_FALSE(!!pIo3);

		// - write to the two valid sockets
		pIo1->write(test::BufferToPacketPayload(sendBuffers[0]), EmptyWriteCallback);
		pIo2->write(test::BufferToPacketPayload(sendBuffers[1]), EmptyWriteCallback);

		// - verify read data (pIo2 should not reference the same socket as pIo1)
		auto pNumReads = CreateCounterPointer();
		state.ClientSockets[0]->read(HandleSocketReadInSendTests(pNumReads, sendBuffers[0]));
		state.ClientSockets[1]->read(HandleSocketReadInSendTests(pNumReads, sendBuffers[1]));

		// - wait for the data to be read
		WAIT_FOR_VALUE(2u, *pNumReads);

		// - 2 sockets are checked out, so none are available
		EXPECT_NUM_ACTIVE_AVAILABLE_WRITERS(2u, 0u, writers);
	}

	TEST(TEST_CLASS, PickOneCanReturnCheckedOutSocketsThatHaveBeenReturned) {
		// Arrange: connect to 2 nodes
		PacketWritersTestContext context(2);
		auto& writers = *context.pWriters;
		auto state = SetupMultiConnectionAcceptTest(context);
		auto sendBuffers = test::GenerateRandomPacketBuffers({ 51, 53 });

		// Act: pick 2 sockets and release 1
		//      (destroying the pIo2 wrapper will signal the completion handler, which will make the socket available again)
		auto pIo1 = writers.pickOne(Default_Timeout).io();
		auto pIo2 = writers.pickOne(Default_Timeout).io();
		pIo2.reset();

		// Sanity: only pIo1 is valid
		ASSERT_TRUE(!!pIo1);
		EXPECT_FALSE(!!pIo2);

		// Act: write a packet to the open socket
		pIo1->write(test::BufferToPacketPayload(sendBuffers[0]), EmptyWriteCallback);

		// - pick a third socket
		//   (use WaitFor because the completion handler triggered by the release of pIo2 is invoked after some
		//    delay because it is posted onto a thread pool)
		decltype(pIo1) pIo3;
		WAIT_FOR_EXPR(!!(pIo3 = writers.pickOne(Default_Timeout).io()));

		// Assert: pIo3 is valid
		ASSERT_TRUE(!!pIo3);

		// - write a packet to it
		pIo3->write(test::BufferToPacketPayload(sendBuffers[1]), EmptyWriteCallback);

		// - verify read data (pIo3 should not reference the same socket as pIo1)
		auto pNumReads = CreateCounterPointer();
		state.ClientSockets[0]->read(HandleSocketReadInSendTests(pNumReads, sendBuffers[0]));
		state.ClientSockets[1]->read(HandleSocketReadInSendTests(pNumReads, sendBuffers[1]));

		// - wait for the data to be read
		WAIT_FOR_VALUE(2u, *pNumReads);

		// - 2 sockets are checked out, so none are available
		EXPECT_NUM_ACTIVE_AVAILABLE_WRITERS(2u, 0u, writers);
	}

	TEST(TEST_CLASS, CanBroadcastPacketOnlyToAvailablePeers) {
		// Arrange: establish multiple connections
		constexpr auto Num_Connections = 5u;
		PacketWritersTestContext context(Num_Connections);
		auto& writers = *context.pWriters;
		auto state = SetupMultiConnectionTest(context);
		MultiConnectionStateGuard stateGuard(writers, state);

		// Act: check out a few peers (0, 1)
		auto pIo1 = writers.pickOne(Default_Timeout).io();
		auto pIo2 = writers.pickOne(Default_Timeout).io();

		// - broadcast a random packet
		auto buffer = test::GenerateRandomPacketBuffer(95);
		writers.broadcast(test::BufferToPacketPayload(buffer));

		// Assert: the broadcast packet was only sent to available peers
		//         the checked out peers received no packets
		auto i = 0u;
		auto pNumReads = CreateCounterPointer();
		for (const auto& pSocket : state.ServerSockets) {
			pSocket->read(HandleSocketReadInSendTests(pNumReads, buffer, i > 1));
			++i;
		}

		// - note that nothing is written to the checked out sockets so they do not update the read counter
		WAIT_FOR_VALUE(Num_Connections - 2, *pNumReads);

		// - all connections are still open but two peers are checked out
		EXPECT_NUM_ACTIVE_AVAILABLE_WRITERS(Num_Connections, Num_Connections - 2, writers);
	}

	TEST(TEST_CLASS, BroadcastDoesNotInvalidateCheckedOutPeers) {
		// Arrange: establish multiple connections
		constexpr auto Num_Connections = 5u;
		PacketWritersTestContext context(Num_Connections);
		auto& writers = *context.pWriters;
		auto state = SetupMultiConnectionTest(context);
		MultiConnectionStateGuard stateGuard(writers, state);
		auto sendBuffers = test::GenerateRandomPacketBuffers({ 51, 52, 95 });

		// Act: check out a few peers (0, 1)
		auto pIo1 = writers.pickOne(Default_Timeout).io();
		auto pIo2 = writers.pickOne(Default_Timeout).io();

		// - broadcast a random packet
		writers.broadcast(test::BufferToPacketPayload(sendBuffers[2]));

		// - write to the checked out peers
		pIo1->write(test::BufferToPacketPayload(sendBuffers[0]), EmptyWriteCallback);
		pIo2->write(test::BufferToPacketPayload(sendBuffers[1]), EmptyWriteCallback);

		// Assert: the broadcast packet was only sent to available peers
		//         the checked out peers received different packets
		auto i = 0u;
		auto pNumReads = CreateCounterPointer();
		for (const auto& pSocket : state.ServerSockets) {
			pSocket->read(HandleSocketReadInSendTests(pNumReads, sendBuffers[std::min<size_t>(i, 2)]));
			++i;
		}

		// - wait for all packets to be read
		WAIT_FOR_VALUE(Num_Connections, *pNumReads);

		// - all connections are still open but two peers are checked out
		EXPECT_NUM_ACTIVE_AVAILABLE_WRITERS(Num_Connections, Num_Connections - 2, writers);
	}

	// endregion

	// region node information (via pickOne)

	TEST(TEST_CLASS, PickOneReturnsNodeInformationWithConnectedSocket) {
		// Act:
		PacketWritersTestContext context;
		RunConnectedSocketTest(context, [&](auto, const auto&) {
			// - pick a pair
			auto packetIoPair = context.pWriters->pickOne(Default_Timeout);

			// Assert: full node information (including endpoint) is available
			ASSERT_TRUE(!!packetIoPair);
			EXPECT_TRUE(!!packetIoPair.io());
			EXPECT_EQ(context.ClientKeyPairs[0].publicKey(), packetIoPair.node().identityKey());
			EXPECT_EQ(test::CreateLocalHostNodeEndpoint().Host, packetIoPair.node().endpoint().Host);
		});
	}

	TEST(TEST_CLASS, PickOneReturnsNodeInformationWithAcceptedSocket) {
		// Act:
		PacketWritersTestContext context;
		RunAcceptedSocketTest(context, [&](auto, const auto&) {
			// - pick a pair
			auto packetIoPair = context.pWriters->pickOne(Default_Timeout);

			// Assert: partial node information (excluding endpoint) is available
			ASSERT_TRUE(!!packetIoPair);
			EXPECT_TRUE(!!packetIoPair.io());
			EXPECT_EQ(context.ClientKeyPairs[0].publicKey(), packetIoPair.node().identityKey());
			EXPECT_TRUE(packetIoPair.node().endpoint().Host.empty());
		});
	}

	// endregion

	// region reconnect

	TEST(TEST_CLASS, CannotConnectToAlreadyConnectedPeer) {
		// Act:
		PacketWritersTestContext context;
		RunConnectedSocketTest(context, [&](auto, const auto&) {
			// Sanity: the connection is active
			EXPECT_NUM_ACTIVE_WRITERS(1u, *context.pWriters);

			// Act: try to connect to the same node again
			PeerConnectResult result;
			auto node = test::CreateLocalHostNode(context.ClientKeyPairs[0].publicKey());
			context.pWriters->connect(node, [&result](const auto& connectResult) {
				result = connectResult;
			});

			// Assert: the connection failed
			EXPECT_EQ(PeerConnectCode::Already_Connected, result.Code);
			EXPECT_EQ(test::ToKeySet(context.ClientKeyPairs), context.pWriters->identities());
		});
	}

	TEST(TEST_CLASS, CannotConnectToAlreadyConnectingPeer) {
		// Act:
		PacketWritersTestContext context;
		RunConnectingSocketTest(context, [&](auto, const auto&) {
			// Sanity: the verifying connection is active
			EXPECT_NUM_PENDING_WRITERS(1u, *context.pWriters);

			// Act: try to connect to the server node again
			PeerConnectResult result;
			context.pWriters->connect(context.serverNode(), [&result](const auto& connectResult) {
				result = connectResult;
			});

			// Assert: the connection failed
			EXPECT_EQ(PeerConnectCode::Already_Connected, result.Code);
			EXPECT_NUM_PENDING_WRITERS(1u, *context.pWriters);
		});
	}

	TEST(TEST_CLASS, CanReconnectToNodeWithInitialConnectFailure) {
		// Arrange: try to connect to a server that isn't running (the connection should fail)
		PacketWritersTestContext context;
		std::atomic<size_t> numCallbacks(0);
		context.pWriters->connect(context.serverNode(), [&](auto) { ++numCallbacks; });
		WAIT_FOR_ONE(numCallbacks);

		// Sanity: no connections were made
		EXPECT_NUM_ACTIVE_WRITERS(0u, *context.pWriters);

		// Act: restart the connection attempt
		RunConnectedSocketTest(context, [&](const auto& connectResult, const auto&) {
			// Assert: the (second) connection attempt succeeded and is active
			EXPECT_EQ(PeerConnectCode::Accepted, connectResult.Code);
			EXPECT_EQ(context.ClientKeyPairs[0].publicKey(), connectResult.IdentityKey);
			EXPECT_NUM_ACTIVE_WRITERS(1u, *context.pWriters);
			EXPECT_EQ(test::ToKeySet(context.ClientKeyPairs), context.pWriters->identities());
		});
	}

	TEST(TEST_CLASS, CanReconnectToDisconnectedNode) {
		// Arrange: create a connection
		PacketWritersTestContext context;
		RunConnectedSocketTest(context, [&](auto, auto& pSocket) {
			// Sanity: the connection is active
			EXPECT_NUM_ACTIVE_WRITERS(1u, *context.pWriters);

			// Act: close the server socket and attempt to read from it (the read should fail)
			pSocket.close();
			context.pWriters->pickOne(Default_Timeout).io()->read(EmptyReadCallback);
			context.waitForConnections(0);

			// Sanity: the connection is closed
			EXPECT_NUM_ACTIVE_WRITERS(0u, *context.pWriters);
		});

		// Act: restart the connection attempt
		CATAPULT_LOG(debug) << "reconnecting";
		RunConnectedSocketTest(context, [&](const auto& connectResult, const auto&) {
			// Assert: the (second) connection attempt succeeded and is active
			EXPECT_EQ(PeerConnectCode::Accepted, connectResult.Code);
			EXPECT_EQ(context.ClientKeyPairs[0].publicKey(), connectResult.IdentityKey);
			EXPECT_NUM_ACTIVE_WRITERS(1u, *context.pWriters);
			EXPECT_EQ(test::ToKeySet(context.ClientKeyPairs), context.pWriters->identities());
		});
	}

	TEST(TEST_CLASS, CanReconnectToDisconnectedNodeAfterShutdown) {
		// Arrange: create a connection
		PacketWritersTestContext context;
		RunConnectedSocketTest(context, [&](auto, const auto&) {
			// Sanity: the connection is active
			EXPECT_NUM_ACTIVE_WRITERS(1u, *context.pWriters);

			// Act: shutdown
			context.pWriters->shutdown();

			// Sanity: the connection is closed
			EXPECT_NUM_ACTIVE_WRITERS(0u, *context.pWriters);
		});

		// Act: restart the connection attempt
		CATAPULT_LOG(debug) << "reconnecting";
		RunConnectedSocketTest(context, [&](const auto& connectResult, const auto&) {
			// Assert: the (second) connection attempt succeeded and is active
			EXPECT_EQ(PeerConnectCode::Accepted, connectResult.Code);
			EXPECT_EQ(context.ClientKeyPairs[0].publicKey(), connectResult.IdentityKey);
			EXPECT_NUM_ACTIVE_WRITERS(1u, *context.pWriters);
			EXPECT_EQ(test::ToKeySet(context.ClientKeyPairs), context.pWriters->identities());
		});
	}

	// endregion

	// region closeOne

	TEST(TEST_CLASS, CloseOneCanCloseConnectedSocket) {
		// Arrange: establish two connections
		PacketWritersTestContext context(2);
		auto state = SetupMultiConnectionTest(context);
		auto& writers = *context.pWriters;

		// Sanity:
		EXPECT_NUM_ACTIVE_WRITERS(2u, writers);

		// Act: close one connection
		auto isClosed = writers.closeOne(context.ClientKeyPairs[0].publicKey());

		// Assert:
		EXPECT_TRUE(isClosed);
		EXPECT_NUM_ACTIVE_WRITERS(1u, writers);
		EXPECT_EQ(utils::KeySet({ context.ClientKeyPairs[1].publicKey() }), writers.identities());
	}

	TEST(TEST_CLASS, CloseOneCanCloseAcceptedSocket) {
		// Arrange: establish two connections
		PacketWritersTestContext context(2);
		auto state = SetupMultiConnectionAcceptTest(context);
		auto& writers = *context.pWriters;

		// Sanity:
		EXPECT_NUM_ACTIVE_WRITERS(2u, writers);

		// Act: close one connection
		auto isClosed = writers.closeOne(context.ClientKeyPairs[0].publicKey());

		// Assert: one writer was destroyed
		EXPECT_TRUE(isClosed);
		EXPECT_EQ(1u, writers.numActiveWriters());
		EXPECT_EQ(utils::KeySet({ context.ClientKeyPairs[1].publicKey() }), writers.identities());

		// Sanity: closing the corresponding server socket removes the pending connection too
		state.ServerSockets[0].reset();
		context.waitForConnections(1);
		EXPECT_NUM_ACTIVE_WRITERS(1u, writers);
	}

	TEST(TEST_CLASS, CloseOneHasNoEffectWhenSpecifiedPeerIsNotConnected) {
		// Arrange: establish two connections
		PacketWritersTestContext context(2);
		auto state = SetupMultiConnectionTest(context);
		auto& writers = *context.pWriters;

		// Sanity:
		EXPECT_NUM_ACTIVE_WRITERS(2u, writers);

		// Act: close one connection
		auto isClosed = writers.closeOne(test::GenerateRandomByteArray<Key>());

		// Assert:
		EXPECT_FALSE(isClosed);
		EXPECT_NUM_ACTIVE_WRITERS(2u, writers);
		EXPECT_EQ(test::ToKeySet(context.ClientKeyPairs), writers.identities());
	}

	// endregion
}}
