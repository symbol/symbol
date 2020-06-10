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
#include "catapult/ionet/BufferedPacketIo.h"
#include "catapult/ionet/Node.h"
#include "catapult/ionet/PacketSocket.h"
#include "catapult/thread/IoThreadPool.h"
#include "catapult/utils/TimeSpan.h"
#include "tests/catapult/net/test/ConnectionContainerTestUtils.h"
#include "tests/test/core/ThreadPoolTestUtils.h"
#include "tests/test/net/NodeTestUtils.h"
#include "tests/test/net/RemoteAcceptServer.h"
#include "tests/test/net/SocketTestUtils.h"
#include "tests/test/nodeps/KeyTestUtils.h"

namespace catapult { namespace net {

#define TEST_CLASS PacketWritersTests

	namespace {
		// region basic test utils / aliases

		static constexpr auto ToIdentity = test::ConnectionContainerTestUtils::ToIdentity;
		static constexpr auto ToIdentitySet = test::ConnectionContainerTestUtils::PublicKeysToIdentitySet;
		static constexpr auto PickIdentities = test::ConnectionContainerTestUtils::PickIdentities;
		static constexpr auto AssertEqualIdentities = test::AssertEqualIdentities;

		const auto Default_Timeout = []() { return utils::TimeSpan::FromMinutes(1); }();

		void EmptyReadCallback(ionet::SocketOperationCode, const ionet::Packet*)
		{}

		void EmptyWriteCallback(ionet::SocketOperationCode)
		{}

		// endregion

		// region PacketWritersTestContext

		struct PacketWritersTestContext {
		public:
			explicit PacketWritersTestContext(size_t numClientPublicKeys = 1)
					: PacketWritersTestContext(numClientPublicKeys, test::GenerateRandomByteArray<Key>())
			{}

			PacketWritersTestContext(size_t numClientPublicKeys, const Key& serverPublicKey)
					: PacketWritersTestContext(numClientPublicKeys, serverPublicKey, test::CreateConnectionSettings(serverPublicKey))
			{}

			PacketWritersTestContext(size_t numClientPublicKeys, const Key& serverPublicKey, const ConnectionSettings& connectionSettings)
					: ServerPublicKey(serverPublicKey)
					, pPool(test::CreateStartedIoThreadPool())
					, IoContext(pPool->ioContext())
					, pWriters(CreatePacketWriters(*pPool, ServerPublicKey, SetRealVerifyCallback(connectionSettings))) {
				for (auto i = 0u; i < numClientPublicKeys; ++i) {
					ClientKeyPairs.push_back(test::GenerateKeyPair());
					ClientPublicKeys.push_back(ClientKeyPairs.back().publicKey());
					Hosts.push_back(std::to_string(i));
				}
			}

			~PacketWritersTestContext() {
				// if a test has already destroyed the writers, just wait for the pool threads to finish
				if (pWriters) {
					pWriters->shutdown();
					test::WaitForUnique(pWriters, "pWriters");
				}

				pPool->join();
			}

		private:
			static ConnectionSettings SetRealVerifyCallback(const ConnectionSettings& settings) {
				auto copy = settings;
				copy.SslOptions.VerifyCallbackSupplier = ionet::CreateSslVerifyCallbackSupplier();
				return copy;
			}

		public:
			Key ServerPublicKey; // the server hosting the PacketWriters instance
			std::vector<Key> ClientPublicKeys; // accepted clients forwarded to the server AND/OR connections initiated by server
			std::vector<crypto::KeyPair> ClientKeyPairs;
			std::vector<std::string> Hosts;
			std::unique_ptr<thread::IoThreadPool> pPool;
			boost::asio::io_context& IoContext;
			std::shared_ptr<PacketWriters> pWriters;

		public:
			ionet::Node serverNode() const {
				// use a client key pair to prevent a self connection error
				return test::CreateLocalHostNode(ClientPublicKeys[0]);
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

		void UseSharedPublicKey(PacketWritersTestContext& context) {
			for (auto& publicKey : context.ClientPublicKeys)
				publicKey = context.ClientPublicKeys[0];
		}

		void UseSharedHost(PacketWritersTestContext& context) {
			for (auto& host : context.Hosts)
				host = context.Hosts[0];
		}

		// endregion

		// region MultiConnectionState

		struct MultiConnectionState {
			std::vector<PeerConnectResult> Results;
			std::vector<std::shared_ptr<ionet::PacketSocket>> ServerSockets; // sockets passed to accept
			std::vector<std::shared_ptr<ionet::PacketSocket>> ClientSockets; // sockets passed to connect (stored in PacketWriters)
		};

		class MultiConnectionStateGuard {
		public:
			MultiConnectionStateGuard(PacketWriters& writers, MultiConnectionState& state)
					: m_writers(writers)
					, m_state(state)
			{}

			~MultiConnectionStateGuard() {
				m_writers.shutdown(); // release all socket references held by the writers
				wait();
			}

		public:
			void wait() {
				// wait for any pending server socket operations to complete
				for (const auto& pSocket : m_state.ServerSockets)
					wait(pSocket);
			}

			void destroyClientSocketAt(size_t index) {
				auto& pSocket = m_state.ClientSockets[index];
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

			auto numConnections = context.ClientPublicKeys.size();
			for (auto i = 0u; i < numConnections; ++i) {
				// - connect to nodes with different identities
				auto peerIdentity = model::NodeIdentity{ context.ClientPublicKeys[i], context.Hosts[i] };
				ionet::Node node(peerIdentity, context.serverNode().endpoint(), ionet::NodeMetadata());

				auto numCallbacks = std::atomic<size_t>(0);
				test::RemoteAcceptServer server(context.ClientKeyPairs[i]);
				server.start(acceptor, [&numCallbacks, &state](const auto& pSocket) {
					state.ServerSockets.push_back(pSocket);
					++numCallbacks;
				});

				context.pWriters->connect(node, [&numCallbacks, &state](const auto& connectResult) {
					state.Results.push_back({ connectResult.Code, connectResult.Identity });
					state.ClientSockets.push_back(connectResult.pPeerSocket);
					++numCallbacks;

					// server callback will not be called when connect returns already connected
					if (PeerConnectCode::Already_Connected == connectResult.Code)
						++numCallbacks;
				});

				// - wait for both connections to complete
				WAIT_FOR_VALUE(2u, numCallbacks);
			}

			context.waitForWriters(0 == numExpectedWriters ? numConnections : numExpectedWriters);
			acceptor.stop();
			WAIT_FOR_EXPR(acceptor.isStopped());
			return state;
		}

		// endregion
	}

	// region custom test macros

#define EXPECT_NUM_ACTIVE_WRITERS(EXPECTED_NUM_WRITERS, WRITERS) \
	do { \
		EXPECT_EQ(EXPECTED_NUM_WRITERS, (WRITERS).numActiveConnections()); \
		EXPECT_EQ(EXPECTED_NUM_WRITERS, (WRITERS).numActiveWriters()); \
		EXPECT_EQ(EXPECTED_NUM_WRITERS, (WRITERS).identities().size()); \
		EXPECT_EQ(EXPECTED_NUM_WRITERS, (WRITERS).numAvailableWriters()); \
	} while (false)

#define EXPECT_NUM_ACTIVE_AVAILABLE_WRITERS(EXPECTED_NUM_WRITERS, EXPECTED_NUM_AVAILABLE_WRITERS, WRITERS) \
	do { \
		EXPECT_EQ(EXPECTED_NUM_WRITERS, (WRITERS).numActiveConnections()); \
		EXPECT_EQ(EXPECTED_NUM_WRITERS, (WRITERS).numActiveWriters()); \
		EXPECT_EQ(EXPECTED_NUM_WRITERS, (WRITERS).identities().size()); \
		EXPECT_EQ(EXPECTED_NUM_AVAILABLE_WRITERS, (WRITERS).numAvailableWriters()); \
	} while (false)

	// endregion

	// region connect failure

	namespace {
		auto CreateDefaultPacketWriters(thread::IoThreadPool& pool) {
			return CreatePacketWriters(pool, Key(), ConnectionSettings());
		}
	}

	TEST(TEST_CLASS, InitiallyNoConnectionsAreActive) {
		// Act:
		auto pPool = test::CreateStartedIoThreadPool();
		auto pWriters = CreateDefaultPacketWriters(*pPool);

		// Assert:
		EXPECT_NUM_ACTIVE_WRITERS(0u, *pWriters);
	}

	TEST(TEST_CLASS, ConnectFailsOnConnectError) {
		// Arrange:
		PacketWritersTestContext context;
		auto numCallbacks = std::atomic<size_t>(0);

		// Act: try to connect to a server that isn't running
		PeerConnectResult result;
		context.pWriters->connect(context.serverNode(), [&numCallbacks, &result](const auto& connectResult) {
			result = connectResult;
			++numCallbacks;
		});
		WAIT_FOR_ONE(numCallbacks);

		// Assert:
		EXPECT_EQ(PeerConnectCode::Socket_Error, result.Code);
		EXPECT_NUM_ACTIVE_WRITERS(0u, *context.pWriters);
	}

	TEST(TEST_CLASS, ConnectFailsOnVerifyError) {
		// Arrange:
		PacketWritersTestContext context;
		auto numCallbacks = std::atomic<size_t>(0);

		// Act: start a server and client verify operation
		PeerConnectResult result;
		test::SpawnPacketServerWork(context.IoContext, [&numCallbacks](const auto& pSocket) {
			// - trigger a verify error by closing the socket without responding
			pSocket->close();
			++numCallbacks;
		});

		context.pWriters->connect(context.serverNode(), [&numCallbacks, &result](const auto& connectResult) {
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

	// endregion

	// region connected writer

	namespace {
		using ResultServerClientHandler = consumer<const PeerConnectResult&, ionet::PacketSocket&>;

		void RunConnectedSocketTest(const PacketWritersTestContext& context, const ResultServerClientHandler& handler) {
			// Act: establish a single connection
			auto state = SetupMultiConnectionTest(context);

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
			EXPECT_EQ(context.ClientPublicKeys[0], connectResult.Identity.PublicKey);
			EXPECT_EQ("0", connectResult.Identity.Host);
			EXPECT_NUM_ACTIVE_WRITERS(1u, *context.pWriters);
			AssertEqualIdentities(ToIdentitySet(context.ClientPublicKeys), context.pWriters->identities());
		});
	}

	namespace {
		template<typename TSetupContext>
		void AssertCanManageMultipleConnections(TSetupContext setupContext) {
			// Act: establish multiple connections
			constexpr auto Num_Connections = 5u;
			PacketWritersTestContext context(Num_Connections);
			auto state = setupContext(context);

			// Assert: all connections are active
			auto i = 0u;
			EXPECT_EQ(Num_Connections, state.Results.size());
			for (const auto& result : state.Results) {
				EXPECT_EQ(PeerConnectCode::Accepted, result.Code);
				EXPECT_EQ(context.ClientPublicKeys[i], result.Identity.PublicKey);
				EXPECT_EQ(std::to_string(i), result.Identity.Host);
				++i;
			}

			EXPECT_NUM_ACTIVE_WRITERS(Num_Connections, *context.pWriters);
			AssertEqualIdentities(ToIdentitySet(context.ClientPublicKeys), context.pWriters->identities());
		}
	}

	TEST(TEST_CLASS, CanManageMultipleConnectionsViaConnect) {
		AssertCanManageMultipleConnections([](auto& context) {
			return SetupMultiConnectionTest(context);
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

	// endregion

	// region unexpected data handling

	namespace {
		template<typename TAction>
		void AssertUnexpectedDataClosesSocket(TAction action) {
			// Arrange: establish a single connection
			PacketWritersTestContext context;
			auto state = SetupMultiConnectionTest(context);

			action(context);

			// Sanity:
			EXPECT_TRUE(test::IsSocketOpen(*state.ClientSockets.back()));

			// Act:
			auto numCallbacks = std::atomic<size_t>(0);
			auto buffer = test::GenerateRandomPacketBuffer(95);
			state.ServerSockets.back()->write(test::BufferToPacketPayload(buffer), [&numCallbacks](auto) { ++numCallbacks; });

			WAIT_FOR_ONE(numCallbacks);
			test::WaitForClosedSocket(*state.ClientSockets.back());

			// Assert:
			EXPECT_FALSE(test::IsSocketOpen(*state.ClientSockets.back()));
		}
	}

	TEST(TEST_CLASS, UnexpectedDataClosesSocket) {
		AssertUnexpectedDataClosesSocket([](const auto&) {});
	}

	TEST(TEST_CLASS, UnexpectedDataAfterPickOneClosesSocket) {
		AssertUnexpectedDataClosesSocket([](const auto& context) {
			// Arrange: get and release the packet io
			auto packetIoPair = context.pWriters->pickOne(Default_Timeout);

			// Sanity:
			EXPECT_EQ(context.ClientPublicKeys[0], packetIoPair.node().identity().PublicKey);
		});
	}

	// endregion

	// region broadcast

	namespace {
		constexpr uint8_t Sentinel_Index = 50;

		auto HandleSocketReadInSendTests(std::atomic<size_t>& counter, const ionet::ByteBuffer& buffer, bool expectSuccess = true) {
			return [&counter, buffer, expectSuccess](auto code, const auto* pPacket) {
				CATAPULT_LOG(debug) << "read from socket returned " << code;

				if (expectSuccess) {
					EXPECT_EQ(ionet::SocketOperationCode::Success, code);

					auto pPacketData = reinterpret_cast<const uint8_t*>(pPacket);
					CATAPULT_LOG(debug)
							<< "read sentinel value " << static_cast<int>(pPacketData[Sentinel_Index])
							<< " (expected " << static_cast<int>(buffer[Sentinel_Index]) << ")";

					EXPECT_EQ(buffer[Sentinel_Index], pPacketData[Sentinel_Index]);
					ASSERT_EQ(buffer.size(), pPacket->Size);
					EXPECT_EQ_MEMORY(&buffer[0], pPacketData, pPacket->Size);
				} else {
					test::AssertSocketClosedDuringRead(code);
					EXPECT_FALSE(!!pPacket);
				}

				++counter;
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
		auto numReads = std::atomic<size_t>(0);
		for (const auto& pSocket : state.ServerSockets)
			pSocket->read(HandleSocketReadInSendTests(numReads, buffer));

		WAIT_FOR_VALUE(Num_Connections, numReads);

		// - all connections are still open
		EXPECT_NUM_ACTIVE_WRITERS(Num_Connections, *context.pWriters);
	}

	TEST(TEST_CLASS, BroadcastClosesPeersThatFail) {
		// Arrange: establish multiple connections
		constexpr auto Num_Connections = 5u;
		PacketWritersTestContext context(Num_Connections);
		auto state = SetupMultiConnectionTest(context);
		MultiConnectionStateGuard stateGuard(*context.pWriters, state);

		// - close two of the client sockets
		state.ClientSockets[2]->close();
		state.ClientSockets[3]->close();

		// Act: broadcast a random packet
		auto buffer = test::GenerateRandomPacketBuffer(95);
		context.pWriters->broadcast(test::BufferToPacketPayload(buffer));

		// Assert: the packet was sent to all connected server sockets
		size_t i = 0;
		auto numReads = std::atomic<size_t>(0);
		for (const auto& pSocket : state.ServerSockets) {
			pSocket->read(HandleSocketReadInSendTests(numReads, buffer, 2 != i && 3 != i));
			++i;
		}

		WAIT_FOR_VALUE(Num_Connections, numReads);

		// - release the shared pointers in state to the closed client sockets (so that the weak pointer cannot be locked)
		stateGuard.destroyClientSocketAt(2);
		stateGuard.destroyClientSocketAt(3);

		// Assert: the closed client sockets have been removed from the active connections
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
			auto numReads = std::atomic<size_t>(0);
			for (const auto& pSocket : state.ServerSockets) {
				auto expectedBuffer = buffer;
				expectedBuffer[Sentinel_Index] = static_cast<uint8_t>(0x80 | (socketId + i * Num_Connections));
				pSocket->read(HandleSocketReadInSendTests(numReads, expectedBuffer));
				++socketId;
			}

			// - wait for all reads and for all connections to be returned
			WAIT_FOR_VALUE(Num_Connections, numReads);
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
			auto numCallbacks = std::atomic<size_t>(0);
			auto readCode = ionet::SocketOperationCode::Success;

			// Act: close the socket and attempt to read from it (the read should fail)
			socket.close();
			writers.pickOne(Default_Timeout).io()->read([&numCallbacks, &readCode](auto code, const auto*) {
				readCode = code;
				++numCallbacks;
			});
			WAIT_FOR_ONE(numCallbacks);

			// Assert: the connection is closed
			test::AssertSocketClosedDuringRead(readCode);
		});
	}

	TEST(TEST_CLASS, PickOneTimeoutClosesPeerAndPreventsSubsequentReads) {
		// Assert: non-deterministic due to timeouts
		AssertNonDeterministicOperationClosesConnectedSocket([](auto& socket, auto& writers, size_t i) {
			// Arrange:
			auto numCallbacks = std::atomic<size_t>(0);
			auto readCode = ionet::SocketOperationCode::Success;

			// - write some dummy data to the socket
			socket.write(test::BufferToPacketPayload(test::GenerateRandomPacketBuffer(95)), EmptyWriteCallback);

			// Act: get an io wrapper, wait for it to timeout, then attempt to read from the io wrapper
			auto pIo = writers.pickOne(utils::TimeSpan::FromMilliseconds(1)).io();
			test::Sleep(static_cast<long>(5 * i));
			pIo->read([&numCallbacks, &readCode](auto code, const auto*) {
				readCode = code;
				++numCallbacks;
			});
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
			auto numCallbacks = std::atomic<size_t>(0);
			auto writeCode = ionet::SocketOperationCode::Success;
			auto pPacket = test::BufferToPacketPayload(test::GenerateRandomPacketBuffer(95));

			// Act: get an io wrapper, shutdown the writers, then attempt to write to the io wrapper
			auto pIo = writers.pickOne(Default_Timeout).io();
			writers.shutdown();
			pIo->write(std::move(pPacket), [&numCallbacks, &writeCode](auto code) {
				writeCode = code;
				++numCallbacks;
			});
			WAIT_FOR_ONE(numCallbacks);

			// Assert: the connection is closed
			EXPECT_EQ(ionet::SocketOperationCode::Write_Error, writeCode);
		});
	}

	TEST(TEST_CLASS, PickOneTimeoutClosesPeerAndPreventsSubsequentWrites) {
		// Assert: non-deterministic due to timeouts
		AssertNonDeterministicOperationClosesConnectedSocket([](const auto&, auto& writers, auto i) {
			// Arrange:
			auto numCallbacks = std::atomic<size_t>(0);
			auto writeCode = ionet::SocketOperationCode::Success;
			auto pPacket = test::BufferToPacketPayload(test::GenerateRandomPacketBuffer(95));

			// Act: get an io wrapper, wait for it to timeout, then attempt to write to the io wrapper
			auto pIo = writers.pickOne(utils::TimeSpan::FromMilliseconds(1)).io();
			test::Sleep(static_cast<long>(5 * i));
			pIo->write(std::move(pPacket), [&numCallbacks, &writeCode](auto code) {
				writeCode = code;
				++numCallbacks;
			});
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
				// Arrange: close the socket and attempt to read from it (the read should fail)
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
			auto numCallbacks = std::atomic<size_t>(0);
			auto readCode = ionet::SocketOperationCode::Success;

			// Act:
			io.read([&numCallbacks, &readCode](auto code, const auto*) {
				readCode = code;
				++numCallbacks;
			});
			WAIT_FOR_ONE(numCallbacks);

			// Assert: the connection is closed
			EXPECT_EQ(ionet::SocketOperationCode::Closed, readCode);
		});
	}

	TEST(TEST_CLASS, PickOneWriteAfterCloseTriggersError) {
		RunTestWithClosedPacketIo([](auto& io) {
			// Arrange:
			auto numCallbacks = std::atomic<size_t>(0);
			auto pPacket = test::BufferToPacketPayload(test::GenerateRandomPacketBuffer(95));
			auto writeCode = ionet::SocketOperationCode::Success;

			// Act:
			io.write(std::move(pPacket), [&numCallbacks, &writeCode](auto code) {
				writeCode = code;
				++numCallbacks;
			});
			WAIT_FOR_ONE(numCallbacks);

			// Assert: the connection is closed
			EXPECT_EQ(ionet::SocketOperationCode::Write_Error, writeCode);
		});
	}

	namespace {
		using IoOperationCallback = consumer<ionet::SocketOperationCode>;
		void AssertOperationCanCompleteWithDestroyedWriters(
				const consumer<ionet::SocketOperationCode>& assertCallbackCode,
				const consumer<ionet::PacketIo&, const IoOperationCallback&>& operation) {
			// Act: create a connection
			PacketWritersTestContext context;
			RunConnectedSocketTest(context, [&](auto, const auto&) {
				// Sanity: the connection is active
				EXPECT_NUM_ACTIVE_WRITERS(1u, *context.pWriters);

				// Arrange:
				auto numCallbacks = std::atomic<size_t>(0);
				auto callbackCode = ionet::SocketOperationCode::Success;

				// - start an operation and immediately destroy the io and shutdown and destroy the writers
				CATAPULT_LOG(debug) << "starting operation";
				auto pIo = context.pWriters->pickOne(Default_Timeout).io();
				operation(*pIo, [&numCallbacks, &callbackCode](auto code) {
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
		AssertOperationCanCompleteWithDestroyedWriters(test::AssertSocketClosedDuringRead, [](auto& io, const auto& callback) {
			io.read([callback](auto code, const auto*) {
				callback(code);
			});
		});
	}

	TEST(TEST_CLASS, PickOneWriteCanCompleteWithDestroyedWriters) {
		AssertOperationCanCompleteWithDestroyedWriters(test::AssertSocketClosedDuringWrite, [](auto& io, const auto& callback) {
			// use a large packet to ensure the operation gets cancelled
			auto pPacket = test::BufferToPacketPayload(test::GenerateRandomPacketBuffer(5 * 1024 * 1024));
			io.write(std::move(pPacket), [callback](auto code) {
				callback(code);
			});
		});
	}

	// endregion

	// region data integrity (broadcast + pickOne)

	namespace {
		using SendFunction = consumer<PacketWriters&, const ionet::PacketPayload&>;
		void RunVerifyWrittenDataTest(size_t numConnections, const SendFunction& send) {
			// Arrange: connect to the specified number of nodes
			auto numPacketsRead = std::atomic<size_t>(0);
			PacketWritersTestContext context(numConnections);
			auto state = SetupMultiConnectionTest(context);

			// Act: send a packet using the supplied function
			auto broadcastBuffer = test::GenerateRandomPacketBuffer(50);
			send(*context.pWriters, test::BufferToPacketPayload(broadcastBuffer));

			// - read from all the server sockets
			size_t socketId = 0;
			std::vector<ionet::ByteBuffer> receiveBuffers(numConnections);
			for (const auto& pSocket : state.ServerSockets) {
				pSocket->read([&numPacketsRead, socketId, &receiveBuffers](auto, const auto* pPacket) {
					auto receiveBuffer = test::CopyPacketToBuffer(*pPacket);
					receiveBuffers[socketId] = receiveBuffer;
					++numPacketsRead;
				});
				++socketId;
			}

			// - wait for all packets to be read
			WAIT_FOR_VALUE(numConnections, numPacketsRead);

			// Assert: the handler was called once for each socket with the same buffer
			for (auto i = 0u; i < numConnections; ++i)
				EXPECT_EQ(broadcastBuffer, receiveBuffers[i]) << "tagged packet " << i;
		}
	}

	TEST(TEST_CLASS, BroadcastDataIsWrittenCorrectlyWhenSingleSocketIsActive) {
		RunVerifyWrittenDataTest(1, [](auto& writers, const auto& payload) { writers.broadcast(payload); });
	}

	TEST(TEST_CLASS, BroadcastDataIsWrittenCorrectlyWhenMultipleSocketsAreActive) {
		RunVerifyWrittenDataTest(4, [](auto& writers, const auto& payload) { writers.broadcast(payload); });
	}

	TEST(TEST_CLASS, PickOneWrapperWritesDataCorrectly) {
		RunVerifyWrittenDataTest(1, [](auto& writers, const auto& payload) {
			writers.pickOne(Default_Timeout).io()->write(payload, EmptyWriteCallback);
		});
	}

	TEST(TEST_CLASS, PickOneWrapperWritesDataCorrectlyWithDestroyedWrapper) {
		RunVerifyWrittenDataTest(1, [](auto& writers, const auto& payload) {
			auto pIo = writers.pickOne(Default_Timeout).io();
			pIo->write(payload, [](auto) { CATAPULT_LOG(debug) << "write completed"; });
			pIo.reset();
			CATAPULT_LOG(debug) << "destroyed wrapper";
		});
	}

	TEST(TEST_CLASS, PickOneWrapperReadsDataCorrectly) {
		// Arrange: connect to a single node
		auto numPacketsRead = std::atomic<size_t>(0);
		PacketWritersTestContext context;
		auto state = SetupMultiConnectionTest(context);

		// - check out the io
		auto pIo = context.pWriters->pickOne(Default_Timeout).io();
		ASSERT_TRUE(!!pIo);

		// - write a packet to the socket (this has to be AFTER pickOne so that the socket is allowed to receive data)
		auto sendBuffer = test::GenerateRandomPacketBuffer(50);
		state.ServerSockets[0]->write(test::BufferToPacketPayload(sendBuffer), EmptyWriteCallback);

		// Act: read the packet using the supplied function
		ionet::ByteBuffer receiveBuffer;
		pIo->read([&numPacketsRead, &receiveBuffer](auto code, const auto* pPacket) {
			if (!pPacket) {
				// test will fail because numPacketsRead is not incremented in this case
				CATAPULT_LOG(warning) << "read failed with " << code;
				return;
			}

			receiveBuffer = test::CopyPacketToBuffer(*pPacket);
			++numPacketsRead;
		});

		// - wait for the packet to be read
		WAIT_FOR_ONE(numPacketsRead);

		// Assert: the expected packet was read
		EXPECT_EQ(sendBuffer, receiveBuffer);
	}

	TEST(TEST_CLASS, PickOneWrapperDestroyedDuringReadDoesNotCauseCrash) {
		// Arrange: connect to a single node
		auto numPacketsRead = std::atomic<size_t>(0);
		PacketWritersTestContext context;
		auto state = SetupMultiConnectionTest(context);

		// - check out the io
		auto pIo = context.pWriters->pickOne(Default_Timeout).io();
		ASSERT_TRUE(!!pIo);

		// - write a packet to the socket (this has to be AFTER pickOne so that the socket is allowed to receive data)
		auto sendBuffer = test::GenerateRandomPacketBuffer(50);
		state.ServerSockets[0]->write(test::BufferToPacketPayload(sendBuffer), EmptyWriteCallback);

		// Act: read the packet using the supplied function
		ionet::ByteBuffer receiveBuffer;
		pIo->read([&numPacketsRead, &receiveBuffer](auto code, const auto* pPacket) {
			if (!pPacket) {
				CATAPULT_LOG(warning) << "read failed with " << code;
				++numPacketsRead;
				return;
			}

			CATAPULT_LOG(debug) << "read completed";
			receiveBuffer = test::CopyPacketToBuffer(*pPacket);
			++numPacketsRead;
		});

		// - destroy the wrapper
		pIo.reset();
		CATAPULT_LOG(debug) << "destroyed wrapper";

		// - wait for the packet to be read
		WAIT_FOR_ONE(numPacketsRead);

		// Assert: check the packet only if it was read
		if (!receiveBuffer.empty())
			EXPECT_EQ(sendBuffer, receiveBuffer);
	}

	// endregion

	// region checkout (broadcast + pickOne)

	TEST(TEST_CLASS, PickOneDecrementsNumAvailableWriters) {
		// Arrange: connect to 3 nodes
		PacketWritersTestContext context(3);
		auto& writers = *context.pWriters;
		auto state = SetupMultiConnectionTest(context);

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
		auto state = SetupMultiConnectionTest(context);

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
		auto state = SetupMultiConnectionTest(context);
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
		auto numReads = std::atomic<size_t>(0);
		state.ServerSockets[0]->read(HandleSocketReadInSendTests(numReads, sendBuffers[0]));
		state.ServerSockets[1]->read(HandleSocketReadInSendTests(numReads, sendBuffers[1]));

		// - wait for the data to be read
		WAIT_FOR_VALUE(2u, numReads);

		// - 2 sockets are checked out, so none are available
		EXPECT_NUM_ACTIVE_AVAILABLE_WRITERS(2u, 0u, writers);
	}

	TEST(TEST_CLASS, PickOneCanReturnCheckedOutSocketsThatHaveBeenReturned) {
		// Arrange: connect to 2 nodes
		PacketWritersTestContext context(2);
		auto& writers = *context.pWriters;
		auto state = SetupMultiConnectionTest(context);
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
		auto numReads = std::atomic<size_t>(0);
		state.ServerSockets[0]->read(HandleSocketReadInSendTests(numReads, sendBuffers[0]));
		state.ServerSockets[1]->read(HandleSocketReadInSendTests(numReads, sendBuffers[1]));

		// - wait for the data to be read
		WAIT_FOR_VALUE(2u, numReads);

		// - 2 sockets are checked out, so none are available
		EXPECT_NUM_ACTIVE_AVAILABLE_WRITERS(2u, 0u, writers);
	}

	TEST(TEST_CLASS, CanBroadcastPacketOnlyToAvailablePeers) {
		// Arrange: keep numReads alive because counter is incremented during writers destruction by reads queued for unavailable peers
		constexpr auto Num_Connections = 5u;
		auto numReads = std::atomic<size_t>(0);

		{
			// - establish multiple connections
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
			for (const auto& pSocket : state.ServerSockets) {
				pSocket->read(HandleSocketReadInSendTests(numReads, buffer, i > 1));
				++i;
			}

			// - note that nothing is written to the checked out sockets so they do not update the read counter
			WAIT_FOR_VALUE(Num_Connections - 2, numReads);

			// - all connections are still open but two peers are checked out
			EXPECT_NUM_ACTIVE_AVAILABLE_WRITERS(Num_Connections, Num_Connections - 2, writers);
		}

		// Sanity:
		WAIT_FOR_VALUE(Num_Connections, numReads);
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
		auto numReads = std::atomic<size_t>(0);
		for (const auto& pSocket : state.ServerSockets) {
			pSocket->read(HandleSocketReadInSendTests(numReads, sendBuffers[std::min<size_t>(i, 2)]));
			++i;
		}

		// - wait for all packets to be read
		WAIT_FOR_VALUE(Num_Connections, numReads);

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
			EXPECT_EQ(context.ClientPublicKeys[0], packetIoPair.node().identity().PublicKey);
			EXPECT_EQ("0", packetIoPair.node().identity().Host);
			EXPECT_EQ(test::CreateLocalHostNodeEndpoint().Host, packetIoPair.node().endpoint().Host);
		});
	}

	namespace {
		void AssertSingleConnection(
				model::NodeIdentityEqualityStrategy equalityStrategy,
				const consumer<PacketWritersTestContext&>& prepare,
				const std::function<model::NodeIdentitySet (const PacketWritersTestContext&)>& extractExpectedIdentities) {
			// Act: establish multiple connections with the same identity
			constexpr auto Num_Connections = 5u;
			auto serverPublicKey = test::GenerateRandomByteArray<Key>();
			auto settings = test::CreateConnectionSettings(serverPublicKey);
			settings.NodeIdentityEqualityStrategy = equalityStrategy;

			PacketWritersTestContext context(Num_Connections, serverPublicKey, settings);
			prepare(context);

			auto state = SetupMultiConnectionTest(context, 1);

			// Assert: all connections succeeded but only a single one is active
			EXPECT_EQ(Num_Connections, state.Results.size());
			EXPECT_EQ(PeerConnectCode::Accepted, state.Results[0].Code);
			for (auto i = 1u; i < state.Results.size(); ++i)
				EXPECT_EQ(PeerConnectCode::Already_Connected, state.Results[i].Code) << "result at " << i;

			EXPECT_EQ(1u, context.pWriters->numActiveConnections());
			EXPECT_EQ(1u, context.pWriters->numActiveWriters());
			AssertEqualIdentities(extractExpectedIdentities(context), context.pWriters->identities());

			EXPECT_NUM_ACTIVE_WRITERS(1u, *context.pWriters);
		}
	}

	TEST(TEST_CLASS, OnlyOneConnectionIsAllowedPerIdentity_KeyPrimacy) {
		AssertSingleConnection(model::NodeIdentityEqualityStrategy::Key, UseSharedPublicKey, [](const auto& context) {
			return ToIdentitySet(context.ClientPublicKeys);
		});
	}

	TEST(TEST_CLASS, OnlyOneConnectionIsAllowedPerIdentity_HostPrimacy) {
		AssertSingleConnection(model::NodeIdentityEqualityStrategy::Host, UseSharedHost, [](const auto& context) {
			return test::ConnectionContainerTestUtils::HostsToIdentitySet(context.Hosts, context.ClientPublicKeys[0]);
		});
	}

	// endregion

	// region reconnect

	namespace {
		void AssertCannotConnectToAlreadyConnectedPeer(
				model::NodeIdentityEqualityStrategy equalityStrategy,
				const std::function<ionet::Node (const PacketWritersTestContext&)>& createNode,
				const std::function<model::NodeIdentitySet (const PacketWritersTestContext&)>& extractExpectedIdentities) {
			// Arrange:
			auto serverPublicKey = test::GenerateRandomByteArray<Key>();
			auto settings = test::CreateConnectionSettings(serverPublicKey);
			settings.NodeIdentityEqualityStrategy = equalityStrategy;

			PacketWritersTestContext context(1, serverPublicKey, settings);
			context.Hosts[0] = "127.0.0.1";

			// Act:
			RunConnectedSocketTest(context, [&](auto, const auto&) {
				// Sanity: the connection is active
				EXPECT_NUM_ACTIVE_WRITERS(1u, *context.pWriters);

				// Act: try to connect to the same node again
				PeerConnectResult result;
				auto node = createNode(context);
				context.pWriters->connect(node, [&result](const auto& connectResult) {
					result = connectResult;
				});

				// Assert: the connection failed
				EXPECT_EQ(PeerConnectCode::Already_Connected, result.Code);
				AssertEqualIdentities(extractExpectedIdentities(context), context.pWriters->identities());
			});
		}
	}

	TEST(TEST_CLASS, CannotConnectToAlreadyConnectedPeer_KeyPrimacy) {
		auto createNode = [](const auto& context) { return test::CreateLocalHostNode(context.ClientPublicKeys[0]); };
		AssertCannotConnectToAlreadyConnectedPeer(model::NodeIdentityEqualityStrategy::Key, createNode, [](const auto& context) {
			return ToIdentitySet(context.ClientPublicKeys);
		});
	}

	TEST(TEST_CLASS, CannotConnectToAlreadyConnectedPeer_HostPrimacy) {
		auto createNode = [](const auto&) { return test::CreateLocalHostNode(test::GenerateRandomByteArray<Key>()); };
		AssertCannotConnectToAlreadyConnectedPeer(model::NodeIdentityEqualityStrategy::Host, createNode, [](const auto& context) {
			return test::ConnectionContainerTestUtils::HostsToIdentitySet(context.Hosts, context.ClientPublicKeys[0]);
		});
	}

	TEST(TEST_CLASS, CanReconnectToNodeWithInitialConnectFailure) {
		// Arrange: try to connect to a server that isn't running (the connection should fail)
		PacketWritersTestContext context;
		auto numCallbacks = std::atomic<size_t>(0);
		context.pWriters->connect(context.serverNode(), [&numCallbacks](const auto&) { ++numCallbacks; });
		WAIT_FOR_ONE(numCallbacks);

		// Sanity: no connections were made
		EXPECT_NUM_ACTIVE_WRITERS(0u, *context.pWriters);

		// Act: restart the connection attempt
		RunConnectedSocketTest(context, [&](const auto& connectResult, const auto&) {
			// Assert: the (second) connection attempt succeeded and is active
			EXPECT_EQ(PeerConnectCode::Accepted, connectResult.Code);
			EXPECT_EQ(context.ClientPublicKeys[0], connectResult.Identity.PublicKey);
			EXPECT_EQ("0", connectResult.Identity.Host);
			EXPECT_NUM_ACTIVE_WRITERS(1u, *context.pWriters);
			AssertEqualIdentities(ToIdentitySet(context.ClientPublicKeys), context.pWriters->identities());
		});
	}

	TEST(TEST_CLASS, CanReconnectToDisconnectedNode) {
		// Arrange: create a connection
		PacketWritersTestContext context;
		RunConnectedSocketTest(context, [&](auto, auto& pSocket) {
			// Sanity: the connection is active
			EXPECT_NUM_ACTIVE_WRITERS(1u, *context.pWriters);

			// Act: close the socket and attempt to read from it (the read should fail)
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
			EXPECT_EQ(context.ClientPublicKeys[0], connectResult.Identity.PublicKey);
			EXPECT_EQ("0", connectResult.Identity.Host);
			EXPECT_NUM_ACTIVE_WRITERS(1u, *context.pWriters);
			AssertEqualIdentities(ToIdentitySet(context.ClientPublicKeys), context.pWriters->identities());
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
			EXPECT_EQ(context.ClientPublicKeys[0], connectResult.Identity.PublicKey);
			EXPECT_EQ("0", connectResult.Identity.Host);
			EXPECT_NUM_ACTIVE_WRITERS(1u, *context.pWriters);
			AssertEqualIdentities(ToIdentitySet(context.ClientPublicKeys), context.pWriters->identities());
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
		auto isClosed = writers.closeOne(ToIdentity(context.ClientPublicKeys[0]));

		// Assert:
		EXPECT_TRUE(isClosed);
		EXPECT_NUM_ACTIVE_WRITERS(1u, writers);
		AssertEqualIdentities(PickIdentities(context.ClientPublicKeys, { 1 }), writers.identities());
	}

	TEST(TEST_CLASS, CloseOneHasNoEffectWhenSpecifiedPeerIsNotConnected) {
		// Arrange: establish two connections
		PacketWritersTestContext context(2);
		auto state = SetupMultiConnectionTest(context);
		auto& writers = *context.pWriters;

		// Sanity:
		EXPECT_NUM_ACTIVE_WRITERS(2u, writers);

		// Act: close one connection
		auto isClosed = writers.closeOne(ToIdentity(test::GenerateRandomByteArray<Key>()));

		// Assert:
		EXPECT_FALSE(isClosed);
		EXPECT_NUM_ACTIVE_WRITERS(2u, writers);
		AssertEqualIdentities(ToIdentitySet(context.ClientPublicKeys), writers.identities());
	}

	// endregion
}}
