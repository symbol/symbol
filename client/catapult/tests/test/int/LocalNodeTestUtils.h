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

#pragma once
#include "catapult/ionet/ConnectResult.h"
#include "catapult/ionet/PacketSocket.h"
#include "catapult/local/BootedLocalNode.h"
#include "catapult/utils/BitwiseEnum.h"
#include "tests/test/core/ThreadPoolTestUtils.h"
#include "tests/test/local/LocalTestUtils.h"
#include "tests/test/local/NetworkTestUtils.h"
#include "tests/test/net/NodeTestUtils.h"
#include "tests/test/net/SocketTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace test {

	/// Local node port.
	constexpr unsigned short Local_Node_Port = Local_Host_Port;

	/// Local node api port.
	constexpr unsigned short Local_Node_Api_Port = Local_Node_Port + 1;

	/// Possible node flags.
	enum class NodeFlag {
		/// A node with a single (self) peer.
		Regular = utils::to_underlying_type(LocalNodeFlags::None),

		/// A node with auto harvesting.
		Auto_Harvest = utils::to_underlying_type(LocalNodeFlags::Should_Auto_Harvest),

		/// A node with custom peers.
		Custom_Peers = 4,

		/// A node with a partner node.
		With_Partner = 8,

		/// A simulated api node.
		Simulated_Api = 16
	};

	MAKE_BITWISE_ENUM(NodeFlag)

	// region counter -> stats adapter

	/// Basic statistics about a local node.
	struct BasicLocalNodeStats {
		/// Number of active packet readers.
		uint64_t NumActiveReaders;

		/// Number of active packet writers.
		uint64_t NumActiveWriters;

		/// Number of scheduled tasks.
		uint64_t NumScheduledTasks;

		/// Number of block elements added to the disruptor.
		uint64_t NumAddedBlockElements;

		/// Number of transaction elements added to the disruptor.
		uint64_t NumAddedTransactionElements;
	};

	/// Statistics about a local p2p node.
	struct PeerLocalNodeStats : public BasicLocalNodeStats {
		/// Number of active broadcast packet writers.
		uint64_t NumActiveBroadcastWriters;

		/// Number of unlocked accounts.
		uint64_t NumUnlockedAccounts;
	};

	/// Returns \c true if \a counters contains a counter with \a name.
	bool HasCounter(const local::LocalNodeCounterValues& counters, const std::string& name);

	/// Extracts all basic statistics from \a counters.
	BasicLocalNodeStats CountersToBasicLocalNodeStats(const local::LocalNodeCounterValues& counters);

	/// Extracts all basic and peer statistics from \a counters.
	PeerLocalNodeStats CountersToPeerLocalNodeStats(const local::LocalNodeCounterValues& counters);

	// endregion

	// region partner nodes

	/// Returns partner server key pair.
	crypto::KeyPair LoadPartnerServerKeyPair();

	/// Creates a local partner node.
	ionet::Node CreateLocalPartnerNode();

	/// Boots a local partner node around \a dataDirectory with \a keyPair.
	std::unique_ptr<local::BootedLocalNode> BootLocalPartnerNode(const std::string& dataDirectory, const crypto::KeyPair& keyPair);

	// endregion

	// region basic tests

	/// Asserts that the local node can be booted without peers and then calls \a handler.
	template<typename TTestContext, typename THandlerFunc>
	void AssertCanBootLocalNodeWithoutPeers(THandlerFunc handler) {
		// Act: create the node with no peers (really it is a peer to itself)
		TTestContext context(NodeFlag::Custom_Peers, {});

		context.waitForNumActiveReaders(0);
		auto stats = context.stats();
		auto nodes = context.localNode().nodes();

		// Assert: check stats
		EXPECT_EQ(0u, stats.NumActiveReaders);
		EXPECT_EQ(0u, stats.NumActiveWriters);
		handler(context, stats);

		// - check nodes
		EXPECT_EQ(1u, nodes.size());
		auto expectedContents = BasicNodeDataContainer{ { LoadServerKeyPair().publicKey(), "LOCAL", ionet::NodeSource::Local } };
		EXPECT_EQ(expectedContents, test::CollectAll(nodes));
	}

	/// Asserts that the local node can be booted with peers and then calls \a handler.
	template<typename TTestContext, typename THandlerFunc>
	void AssertCanBootLocalNodeWithPeers(THandlerFunc handler) {
		// Act: create the node with custom peers
		auto keys = GenerateRandomDataVector<Key>(3);
		TTestContext context(NodeFlag::Custom_Peers | NodeFlag::With_Partner, {
			CreateNamedNode(keys[0], "alice"),
			CreateNamedNode(keys[1], "bob"),
			CreateNamedNode(keys[2], "charlie"),
			CreateLocalPartnerNode()
		});

		context.waitForNumActiveWriters(1);
		auto stats = context.stats();
		auto nodes = context.localNode().nodes();

		// Assert: check stats
		EXPECT_EQ(0u, stats.NumActiveReaders);
		EXPECT_EQ(1u, stats.NumActiveWriters);
		handler(context, stats);

		// - check nodes
		EXPECT_EQ(5u, nodes.size());
		auto expectedContents = BasicNodeDataContainer{
			{ LoadServerKeyPair().publicKey(), "LOCAL", ionet::NodeSource::Local },
			{ CreateLocalPartnerNode().identityKey(), "PARTNER", ionet::NodeSource::Static },
			{ keys[0], "alice", ionet::NodeSource::Static },
			{ keys[1], "bob", ionet::NodeSource::Static },
			{ keys[2], "charlie", ionet::NodeSource::Static }
		};
		EXPECT_EQ(expectedContents, test::CollectAll(nodes));
	}

	/// Asserts that the local node can be shutdown.
	template<typename TTestContext>
	void AssertCanShutdownLocalNode() {
		// Arrange:
		TTestContext context(NodeFlag::Regular);

		// Act:
		context.localNode().shutdown();

		// Assert:
		context.assertShutdown();
	}

	/// Asserts that the local node is booted with \a numTasks scheduled tasks.
	template<typename TTestContext>
	void AssertLocalNodeSchedulesTasks(uint32_t numTasks) {
		// Act:
		TTestContext context(NodeFlag::Regular);

		context.waitForNumScheduledTasks(numTasks);
		auto stats = context.stats();

		// Assert:
		EXPECT_EQ(numTasks, stats.NumScheduledTasks);
	}

	// endregion

	// region connection tests

	/// Asserts that the local node cannot be connected to on \a port.
	template<typename TTestContext>
	void AssertConnectionError(unsigned short port) {
		// Arrange: boot a local node
		TTestContext context(NodeFlag::Regular);

		// Act: attempt to connect to the node
		auto result = ionet::ConnectResult::Connected;
		std::atomic_bool isConnectionAttemptComplete(false);
		auto pPool = CreateStartedIoServiceThreadPool(1);
		auto options = CreatePacketSocketOptions();
		auto endpoint = CreateLocalHostNodeEndpoint(port);
		auto clientKeyPair = GenerateKeyPair();
		ionet::Connect(pPool->service(), options, endpoint, [&](auto connectResult, const auto&) {
			CATAPULT_LOG(debug) << "connection attempt completed with " << connectResult;
			result = connectResult;
			isConnectionAttemptComplete = true;
		});
		WAIT_FOR(isConnectionAttemptComplete);

		// Assert: the connection could not be established
		EXPECT_EQ(ionet::ConnectResult::Connect_Error, result);
	}

	/// Represents an external connection.
	struct ExternalConnection {
		/// Connection thread pool.
		std::unique_ptr<thread::IoServiceThreadPool> pPool;

		/// Connection io.
		std::shared_ptr<ionet::PacketIo> pIo;
	};

	/// Creates an external connection to the local node on \a port.
	ExternalConnection CreateExternalConnection(unsigned short port);

	// endregion
}}
