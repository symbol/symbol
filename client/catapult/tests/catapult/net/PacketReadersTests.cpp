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

#include "catapult/net/PacketReaders.h"
#include "catapult/ionet/PacketSocket.h"
#include "catapult/ionet/SocketReader.h"
#include "catapult/thread/IoThreadPool.h"
#include "tests/catapult/net/test/ConnectionContainerTestUtils.h"
#include "tests/test/core/ThreadPoolTestUtils.h"
#include "tests/test/net/NodeTestUtils.h"
#include "tests/test/net/SocketTestUtils.h"
#include "tests/test/nodeps/KeyTestUtils.h"
#include <unordered_map>

namespace catapult { namespace net {

#define TEST_CLASS PacketReadersTests

	namespace {
		// region basic test utils / aliases

		static constexpr auto ToIdentity = test::ConnectionContainerTestUtils::ToIdentity;
		static constexpr auto ToIdentitySet = test::ConnectionContainerTestUtils::PublicKeysToIdentitySet;
		static constexpr auto PickIdentities = test::ConnectionContainerTestUtils::PickIdentities;
		static constexpr auto AssertEqualIdentities = test::AssertEqualIdentities;

		// endregion

		// region PacketReadersTestContext

		struct PacketReadersTestContext {
		public:
			explicit PacketReadersTestContext(uint32_t numClientPublicKeys = 1, uint32_t maxConnectionsPerIdentity = 1)
					: PacketReadersTestContext(ionet::ServerPacketHandlers(), numClientPublicKeys, maxConnectionsPerIdentity)
			{}

			PacketReadersTestContext(
					const ionet::ServerPacketHandlers& handlers,
					uint32_t numClientPublicKeys,
					uint32_t maxConnectionsPerIdentity,
					const ConnectionSettings& connectionSettings = ConnectionSettings())
					: ServerPublicKey(test::GenerateRandomByteArray<Key>())
					, pPool(test::CreateStartedIoThreadPool())
					, IoContext(pPool->ioContext())
					, Handlers(handlers)
					, pReaders(CreatePacketReaders(*pPool, Handlers, ServerPublicKey, connectionSettings, maxConnectionsPerIdentity)) {
				for (auto i = 0u; i < numClientPublicKeys; ++i) {
					ClientPublicKeys.push_back(test::GenerateRandomByteArray<Key>());
					Hosts.push_back(std::to_string(i));
				}
			}

			~PacketReadersTestContext() {
				pReaders->shutdown();
				test::WaitForUnique(pReaders, "pReaders");
				pPool->join();
			}

		public:
			Key ServerPublicKey; // the server hosting the PacketReaders instance
			std::vector<Key> ClientPublicKeys; // accepted clients forwarded to the server
			std::vector<std::string> Hosts;
			std::unique_ptr<thread::IoThreadPool> pPool;
			boost::asio::io_context& IoContext;
			ionet::ServerPacketHandlers Handlers;
			std::shared_ptr<PacketReaders> pReaders;

		public:
			void waitForConnections(size_t numConnections) const {
				WAIT_FOR_VALUE_EXPR(numConnections, pReaders->numActiveConnections());
			}

			void waitForReaders(size_t numReaders) const {
				WAIT_FOR_VALUE_EXPR(numReaders, pReaders->numActiveReaders());
			}
		};

		void UseSharedPublicKey(PacketReadersTestContext& context) {
			for (auto& publicKey : context.ClientPublicKeys)
				publicKey = context.ClientPublicKeys[0];
		}

		void UseSharedHost(PacketReadersTestContext& context) {
			for (auto& host : context.Hosts)
				host = context.Hosts[0];
		}

		// endregion

		// region MultiConnectionState

		struct MultiConnectionState {
			std::vector<PeerConnectResult> Results;
			std::vector<std::shared_ptr<ionet::PacketSocket>> ServerSockets;
			std::vector<std::shared_ptr<ionet::PacketSocket>> ClientSockets;
		};

		MultiConnectionState SetupMultiConnectionTest(const PacketReadersTestContext& context, bool userCallbackResult = true) {
			// Act: start multiple server and client verify operations
			MultiConnectionState state;
			test::TcpAcceptor acceptor(context.IoContext);
			for (auto i = 0u; i < context.ClientPublicKeys.size(); ++i) {
				std::atomic<size_t> numCallbacks(0);
				auto host = context.Hosts[i];
				auto clientPublicKey = context.ClientPublicKeys[i];
				test::SpawnPacketServerWork(acceptor, [&, host, clientPublicKey](const auto& pSocket) {
					state.ServerSockets.push_back(pSocket);

					auto packetSocketInfo = ionet::PacketSocketInfo(host, clientPublicKey, pSocket);
					context.pReaders->accept(packetSocketInfo, [&state, &numCallbacks, userCallbackResult](const auto& connectResult) {
						state.Results.push_back(connectResult);
						++numCallbacks;
						return userCallbackResult;
					});
				});

				test::SpawnPacketClientWork(context.IoContext, [&state, &numCallbacks](const auto& pSocket) {
					state.ClientSockets.push_back(pSocket);
					++numCallbacks;
				});

				// - wait for both connections to complete
				WAIT_FOR_VALUE(2u, numCallbacks);
			}

			return state;
		}

		// endregion
	}

	// region custom test macros

#define EXPECT_NUM_ACTIVE_READERS(EXPECTED_NUM_READERS, READERS) \
	do { \
		EXPECT_EQ(EXPECTED_NUM_READERS, (READERS).numActiveConnections()); \
		EXPECT_EQ(EXPECTED_NUM_READERS, (READERS).numActiveReaders()); \
		EXPECT_EQ(EXPECTED_NUM_READERS, (READERS).identities().size()); \
	} while (false)

#define EXPECT_NUM_ACTIVE_READERS_AND_IDENTITIES(EXPECTED_NUM_READERS, EXPECTED_NUM_IDENTITIES, READERS) \
	do { \
		EXPECT_EQ(EXPECTED_NUM_READERS, (READERS).numActiveConnections()); \
		EXPECT_EQ(EXPECTED_NUM_READERS, (READERS).numActiveReaders()); \
		EXPECT_EQ(EXPECTED_NUM_IDENTITIES, (READERS).identities().size()); \
	} while (false)

	// endregion

	// region accept failure

	namespace {
		auto CreateDefaultPacketReaders(thread::IoThreadPool& pool) {
			return CreatePacketReaders(pool, ionet::ServerPacketHandlers(), Key(), ConnectionSettings(), 1);
		}
	}

	TEST(TEST_CLASS, InitiallyNoConnectionsAreActive) {
		// Act:
		auto pPool = test::CreateStartedIoThreadPool();
		auto pReaders = CreateDefaultPacketReaders(*pPool);

		// Assert:
		EXPECT_NUM_ACTIVE_READERS(0u, *pReaders);
	}

	TEST(TEST_CLASS, AcceptFailsOnAcceptError) {
		// Arrange:
		auto pPool = test::CreateStartedIoThreadPool();
		auto pReaders = CreateDefaultPacketReaders(*pPool);

		// Act: on an accept error, the server will pass nullptr
		PeerConnectResult result;
		pReaders->accept(ionet::PacketSocketInfo(), [&result](const auto& acceptResult) {
			result = acceptResult;
			return true;
		});

		// Assert:
		EXPECT_EQ(PeerConnectCode::Socket_Error, result.Code);
		EXPECT_NUM_ACTIVE_READERS(0u, *pReaders);
	}

	// endregion

	// region accepted reader

	namespace {
		using ResultServerClientHandler = consumer<const PeerConnectResult&, ionet::PacketSocket&, ionet::PacketSocket&>;

		void RunConnectedSocketTest(const PacketReadersTestContext& context, const ResultServerClientHandler& handler) {
			// Act: establish a single connection
			auto state = SetupMultiConnectionTest(context);

			// Assert: call the handler
			handler(state.Results.back(), *state.ServerSockets.back(), *state.ClientSockets.back());
		}
	}

	TEST(TEST_CLASS, AcceptSucceedsOnVerifySuccess) {
		// Act:
		PacketReadersTestContext context;
		RunConnectedSocketTest(context, [&](const auto& connectResult, const auto&, const auto&) {
			// Assert: the verification should have succeeded and the connection should be active
			EXPECT_EQ(PeerConnectCode::Accepted, connectResult.Code);
			EXPECT_EQ(context.ClientPublicKeys[0], connectResult.Identity.PublicKey);
			EXPECT_EQ("0", connectResult.Identity.Host);
			EXPECT_NUM_ACTIVE_READERS(1u, *context.pReaders);
			AssertEqualIdentities(ToIdentitySet(context.ClientPublicKeys), context.pReaders->identities());
		});
	}

	TEST(TEST_CLASS, AcceptIsBypassedByCallbackError) {
		// Act: attempt to establish a single connection, but configure user callback to return false
		PacketReadersTestContext context;
		auto state = SetupMultiConnectionTest(context, false);

		// Assert: the verification should have succeeded but the connection should NOT be active
		const auto& connectResult = state.Results.back();
		EXPECT_EQ(PeerConnectCode::Accepted, connectResult.Code);
		EXPECT_EQ(context.ClientPublicKeys[0], connectResult.Identity.PublicKey);
		EXPECT_EQ("0", connectResult.Identity.Host);

		// Sanity: closing the corresponding server sockets removes the pending connections
		state.ServerSockets.clear();
		context.waitForConnections(0);
		EXPECT_NUM_ACTIVE_READERS(0u, *context.pReaders);
	}

	TEST(TEST_CLASS, ShutdownClosesAcceptedSocket) {
		// Act:
		PacketReadersTestContext context;
		RunConnectedSocketTest(context, [&](auto, auto& serverSocket, const auto&) {
			// Act: shutdown the readers
			context.pReaders->shutdown();
			test::WaitForClosedSocket(serverSocket);

			// Assert: the server socket was closed
			EXPECT_FALSE(test::IsSocketOpen(serverSocket));
			EXPECT_NUM_ACTIVE_READERS(0u, *context.pReaders);
		});
	}

	TEST(TEST_CLASS, CanManageMultipleConnections) {
		// Act: establish multiple connections
		constexpr auto Num_Connections = 5u;
		PacketReadersTestContext context(Num_Connections);
		auto state = SetupMultiConnectionTest(context);

		// Assert: all connections are active
		auto i = 0u;
		EXPECT_EQ(Num_Connections, state.Results.size());
		for (const auto& result : state.Results) {
			EXPECT_EQ(PeerConnectCode::Accepted, result.Code);
			EXPECT_EQ(context.ClientPublicKeys[i], result.Identity.PublicKey);
			EXPECT_EQ(std::to_string(i), result.Identity.Host);
			++i;
		}

		EXPECT_NUM_ACTIVE_READERS(Num_Connections, *context.pReaders);
		AssertEqualIdentities(ToIdentitySet(context.ClientPublicKeys), context.pReaders->identities());
	}

	namespace {
		void AssertSingleConnection(
				model::NodeIdentityEqualityStrategy equalityStrategy,
				const consumer<PacketReadersTestContext&>& prepare,
				const std::function<model::NodeIdentitySet (const PacketReadersTestContext&)>& extractExpectedIdentities) {
			// Act: establish multiple connections with the same identity
			constexpr auto Num_Connections = 5u;
			auto settings = test::CreateConnectionSettings();
			settings.NodeIdentityEqualityStrategy = equalityStrategy;

			PacketReadersTestContext context(ionet::ServerPacketHandlers(), Num_Connections, 1, settings);
			prepare(context);

			auto state = SetupMultiConnectionTest(context);

			// Assert: all connections succeeded but only a single one is active
			EXPECT_EQ(Num_Connections, state.Results.size());
			EXPECT_EQ(PeerConnectCode::Accepted, state.Results[0].Code);
			for (auto i = 1u; i < state.Results.size(); ++i)
				EXPECT_EQ(PeerConnectCode::Already_Connected, state.Results[i].Code) << "result at " << i;

			EXPECT_EQ(Num_Connections, context.pReaders->numActiveConnections());
			EXPECT_EQ(1u, context.pReaders->numActiveReaders());
			AssertEqualIdentities(extractExpectedIdentities(context), context.pReaders->identities());

			// Sanity: closing the corresponding server sockets removes the pending connections
			state.ServerSockets.clear();
			context.waitForConnections(1);
			EXPECT_NUM_ACTIVE_READERS(1u, *context.pReaders);
		}

		void AssertMultipleConnections(
				model::NodeIdentityEqualityStrategy equalityStrategy,
				const consumer<PacketReadersTestContext&>& prepare,
				const std::function<model::NodeIdentitySet (const PacketReadersTestContext&)>& extractExpectedIdentities) {
			// Act: establish multiple connections with the same identity
			constexpr auto Num_Connections = 5u;
			auto settings = test::CreateConnectionSettings();
			settings.NodeIdentityEqualityStrategy = equalityStrategy;

			PacketReadersTestContext context(ionet::ServerPacketHandlers(), Num_Connections, 3, settings);
			prepare(context);

			auto state = SetupMultiConnectionTest(context);

			// Assert: all connections succeeded but three are active
			EXPECT_EQ(Num_Connections, state.Results.size());
			EXPECT_EQ(PeerConnectCode::Accepted, state.Results[0].Code);
			EXPECT_EQ(PeerConnectCode::Accepted, state.Results[1].Code);
			EXPECT_EQ(PeerConnectCode::Accepted, state.Results[2].Code);
			for (auto i = 3u; i < state.Results.size(); ++i)
				EXPECT_EQ(PeerConnectCode::Already_Connected, state.Results[i].Code) << "result at " << i;

			EXPECT_EQ(Num_Connections, context.pReaders->numActiveConnections());
			EXPECT_EQ(3u, context.pReaders->numActiveReaders());
			AssertEqualIdentities(extractExpectedIdentities(context), context.pReaders->identities());

			// Sanity: closing the corresponding server sockets removes the pending connections
			state.ServerSockets.clear();
			context.waitForConnections(3);
			EXPECT_NUM_ACTIVE_READERS_AND_IDENTITIES(3u, 1u, *context.pReaders);
		}
	}

	TEST(TEST_CLASS, OnlyOneConnectionIsAllowedPerIdentityByDefault_KeyPrimacy) {
		AssertSingleConnection(model::NodeIdentityEqualityStrategy::Key, UseSharedPublicKey, [](const auto& context) {
			return ToIdentitySet(context.ClientPublicKeys);
		});
	}

	TEST(TEST_CLASS, OnlyOneConnectionIsAllowedPerIdentityByDefault_HostPrimacy) {
		AssertSingleConnection(model::NodeIdentityEqualityStrategy::Host, UseSharedHost, [](const auto& context) {
			return test::ConnectionContainerTestUtils::HostsToIdentitySet(context.Hosts, context.ClientPublicKeys[0]);
		});
	}

	TEST(TEST_CLASS, MultipleConnectionsAreAllowedPerIdentityWithCustomConfiguration_KeyPrimacy) {
		AssertMultipleConnections(model::NodeIdentityEqualityStrategy::Key, UseSharedPublicKey, [](const auto& context) {
			return ToIdentitySet(context.ClientPublicKeys);
		});
	}

	TEST(TEST_CLASS, MultipleConnectionsAreAllowedPerIdentityWithCustomConfiguration_HostPrimacy) {
		AssertMultipleConnections(model::NodeIdentityEqualityStrategy::Host, UseSharedHost, [](const auto& context) {
			return test::ConnectionContainerTestUtils::HostsToIdentitySet(context.Hosts, context.ClientPublicKeys[0]);
		});
	}

	// endregion

	// region read forwarding

	namespace {
		constexpr size_t Tag_Index = sizeof(ionet::Packet);

		ionet::ByteBuffer SendTaggedPacket(ionet::PacketIo& io, uint8_t tag) {
			auto sendBuffer = test::GenerateRandomPacketBuffer(62);
			sendBuffer[Tag_Index] = tag;

			CATAPULT_LOG(debug) << "writing packet " << tag;
			io.write(test::BufferToPacketPayload(sendBuffer), [](auto) {});
			return sendBuffer;
		}

		size_t ReceiveTaggedPacket(const ionet::Packet& packet, std::vector<ionet::ByteBuffer>& receiveBuffers) {
			auto receiveBuffer = test::CopyPacketToBuffer(packet);
			auto tag = receiveBuffer[Tag_Index];

			CATAPULT_LOG(debug) << "handling packet " << tag;
			receiveBuffers[tag] = receiveBuffer;
			return tag;
		}

		void RunPacketHandlerTest(uint32_t numConnections, bool useSharedIdentity) {
			// Arrange: set up a handler that copies packets
			ionet::ServerPacketHandlers handlers;
			std::atomic<size_t> numPacketsRead(0);
			std::vector<ionet::ByteBuffer> receiveBuffers(numConnections);
			std::vector<model::NodeIdentity> receiveIdentities(numConnections);
			test::RegisterDefaultHandler(handlers, [&](const auto& packet, const auto& context) {
				auto tag = ReceiveTaggedPacket(packet, receiveBuffers);
				receiveIdentities[tag] = { context.key(), context.host() };
				++numPacketsRead;
			});

			// - connect to the specified number of nodes
			PacketReadersTestContext context(handlers, numConnections, useSharedIdentity ? numConnections : 1);
			if (useSharedIdentity)
				UseSharedPublicKey(context);

			auto state = SetupMultiConnectionTest(context);

			// Act: send a single (different) packet to each socket and uniquely tag each packet
			std::vector<ionet::ByteBuffer> sendBuffers;
			for (const auto& pSocket : state.ClientSockets)
				sendBuffers.push_back(SendTaggedPacket(*pSocket, static_cast<uint8_t>(sendBuffers.size())));

			// - wait for all packets to be read
			WAIT_FOR_VALUE(numConnections, numPacketsRead);

			// Assert: the handler was called once for each socket with the corresponding sent packet
			for (auto i = 0u; i < numConnections; ++i) {
				auto message = "tagged packet " + std::to_string(i);
				EXPECT_EQ(sendBuffers[i], receiveBuffers[i]) << message;
				EXPECT_EQ(context.ClientPublicKeys[i], receiveIdentities[i].PublicKey) << message;
				EXPECT_EQ(std::to_string(i), receiveIdentities[i].Host) << message;
			}
		}
	}

	TEST(TEST_CLASS, SingleReaderPassesPayloadToHandlers) {
		RunPacketHandlerTest(1, false);
	}

	TEST(TEST_CLASS, MultipleReadersPassPayloadToHandlers) {
		RunPacketHandlerTest(4, false);
	}

	TEST(TEST_CLASS, MultipleReadersWithSharedIdentityPassPayloadToHandlers) {
		RunPacketHandlerTest(4, true);
	}

	// endregion

	// region read error

	TEST(TEST_CLASS, ReadErrorDisconnectsConnectedSocket) {
		// Act: create a connection
		PacketReadersTestContext context;
		auto state = SetupMultiConnectionTest(context);
		auto& readers = *context.pReaders;

		// Sanity: the connection is active
		EXPECT_NUM_ACTIVE_READERS(1u, readers);

		// Act: trigger the operation that should close the socket and wait for the connections to drop
		// (send a packet without a corresponding handler)
		state.ClientSockets[0]->write(test::BufferToPacketPayload(test::GenerateRandomPacketBuffer(62)), [](auto) {});
		context.waitForReaders(0);

		// Assert: the connection is closed
		EXPECT_EQ(0u, readers.numActiveReaders());
		EXPECT_TRUE(readers.identities().empty());

		// Sanity: closing the corresponding server socket removes the pending connection too
		state.ServerSockets[0].reset();
		context.waitForConnections(0);
		EXPECT_NUM_ACTIVE_READERS(0u, readers);
	}

	TEST(TEST_CLASS, ReadErrorDisconnectsAllConnectedSocketsWithSameIdentity) {
		// Act: establish five connections, three of which have the same identity
		PacketReadersTestContext context(5, 3);
		context.ClientPublicKeys[2] = context.ClientPublicKeys[0];
		context.ClientPublicKeys[4] = context.ClientPublicKeys[0];

		auto state = SetupMultiConnectionTest(context);
		auto& readers = *context.pReaders;

		// Sanity:
		EXPECT_NUM_ACTIVE_READERS_AND_IDENTITIES(5u, 3u, readers);

		// Act: trigger the operation that should close the socket and wait for the connections to drop
		// (send a packet without a corresponding handler)
		state.ClientSockets[0]->write(test::BufferToPacketPayload(test::GenerateRandomPacketBuffer(62)), [](auto) {});
		context.waitForReaders(2);

		// Assert: three readers (shared identity) were destroyed
		EXPECT_EQ(2u, readers.numActiveReaders());
		AssertEqualIdentities(PickIdentities(context.ClientPublicKeys, { 1, 3 }), readers.identities());

		// Sanity: closing the corresponding server socket removes the pending connection too
		state.ServerSockets[0].reset();
		state.ServerSockets[2].reset();
		state.ServerSockets[4].reset();
		context.waitForConnections(2);
		EXPECT_NUM_ACTIVE_READERS(2u, *context.pReaders);
	}

	TEST(TEST_CLASS, CloseDoesNotTriggerAnyDisconnectionsOfSocketsWithSameIdentity) {
		// Act: establish five connections, three of which have the same identity
		PacketReadersTestContext context(5, 3);
		context.ClientPublicKeys[2] = context.ClientPublicKeys[0];
		context.ClientPublicKeys[4] = context.ClientPublicKeys[0];

		auto state = SetupMultiConnectionTest(context);
		auto& readers = *context.pReaders;

		// Sanity:
		EXPECT_NUM_ACTIVE_READERS_AND_IDENTITIES(5u, 3u, readers);

		// Act: close one of the sockets
		state.ClientSockets[2]->close();
		context.waitForReaders(4);

		// Assert: one reader (closed one) was destroyed
		EXPECT_EQ(4u, readers.numActiveReaders());
		AssertEqualIdentities(PickIdentities(context.ClientPublicKeys, { 0, 1, 3 }), readers.identities());

		// Sanity: closing the corresponding server socket removes the pending connection too
		state.ServerSockets[2].reset();
		context.waitForConnections(4);
		EXPECT_NUM_ACTIVE_READERS_AND_IDENTITIES(4u, 3u, *context.pReaders);
	}

	// endregion

	// region closeOne

	TEST(TEST_CLASS, CloseOneCanCloseConnectedSocket) {
		// Arrange: establish two connections
		PacketReadersTestContext context(2);
		auto state = SetupMultiConnectionTest(context);
		auto& readers = *context.pReaders;

		// Sanity:
		EXPECT_NUM_ACTIVE_READERS(2u, readers);

		// Act: close one connection
		auto isClosed = readers.closeOne(ToIdentity(context.ClientPublicKeys[0]));

		// Assert: one reader was destroyed
		EXPECT_TRUE(isClosed);
		EXPECT_EQ(1u, readers.numActiveReaders());
		AssertEqualIdentities(PickIdentities(context.ClientPublicKeys, { 1 }), readers.identities());

		// Sanity: closing the corresponding server socket removes the pending connection too
		state.ServerSockets[0].reset();
		context.waitForConnections(1);
		EXPECT_NUM_ACTIVE_READERS(1u, readers);
	}

	TEST(TEST_CLASS, CloseOneHasNoEffectWhenSpecifiedPeerIsNotConnected) {
		// Arrange: establish two connections
		PacketReadersTestContext context(2);
		auto state = SetupMultiConnectionTest(context);
		auto& readers = *context.pReaders;

		// Sanity:
		EXPECT_NUM_ACTIVE_READERS(2u, readers);

		// Act: close one connection
		auto isClosed = readers.closeOne(ToIdentity(test::GenerateRandomByteArray<Key>()));

		// Assert:
		EXPECT_FALSE(isClosed);
		EXPECT_NUM_ACTIVE_READERS(2u, readers);
		AssertEqualIdentities(ToIdentitySet(context.ClientPublicKeys), readers.identities());
	}

	TEST(TEST_CLASS, CloseOneCanCloseAllSocketsWithMatchingIdentity) {
		// Act: establish five connections, three of which have the same identity
		PacketReadersTestContext context(5, 3);
		context.ClientPublicKeys[2] = context.ClientPublicKeys[0];
		context.ClientPublicKeys[4] = context.ClientPublicKeys[0];

		auto state = SetupMultiConnectionTest(context);
		auto& readers = *context.pReaders;

		// Sanity:
		EXPECT_NUM_ACTIVE_READERS_AND_IDENTITIES(5u, 3u, readers);

		// Act: close one connection
		auto isClosed = readers.closeOne(ToIdentity(context.ClientPublicKeys[2]));

		// Assert: three readers (shared identity) were destroyed
		EXPECT_TRUE(isClosed);
		EXPECT_EQ(2u, readers.numActiveReaders());
		AssertEqualIdentities(PickIdentities(context.ClientPublicKeys, { 1, 3 }), readers.identities());

		// Sanity: closing the corresponding server socket removes the pending connection too
		state.ServerSockets[0].reset();
		state.ServerSockets[2].reset();
		state.ServerSockets[4].reset();
		context.waitForConnections(2);
		EXPECT_NUM_ACTIVE_READERS(2u, *context.pReaders);
	}

	// endregion
}}
