#pragma once
#include "catapult/ionet/ConnectResult.h"
#include "catapult/ionet/PacketSocket.h"
#include "catapult/local/BootedLocalNode.h"
#include "tests/test/core/ThreadPoolTestUtils.h"
#include "tests/test/local/LocalTestUtils.h"
#include "tests/test/local/NetworkTestUtils.h"
#include "tests/test/net/SocketTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace test {

	/// The local node port.
	constexpr unsigned short Local_Node_Port = Local_Host_Port;

	/// The local node api port.
	constexpr unsigned short Local_Node_Api_Port = Local_Node_Port + 1;

	/// Possible node flags.
	enum class NodeFlag {
		/// A node with peers.
		Regular = local_node_flags::None,

		/// A node with auto harvesting.
		Auto_Harvest = local_node_flags::Should_Auto_Harvest,

		/// A node without any peers.
		No_Peers = local_node_flags::No_Peers,
	};

	// region basic tests

	/// Asserts that the local node can be booted without peers and then calls \a handler.
	template<typename TTestContext, typename THandlerFunc>
	void AssertCanBootLocalNodeWithoutPeers(THandlerFunc handler) {
		// Act:
		TTestContext context(NodeFlag::No_Peers);

		context.waitForNumActiveReaders(0);
		auto stats = context.stats();

		// Assert:
		EXPECT_EQ(0u, stats.NumActiveReaders);
		EXPECT_EQ(0u, stats.NumActiveWriters);
		handler(context, stats);
	}

	/// Asserts that the local node can be booted with peers and then calls \a handler.
	template<typename TTestContext, typename THandlerFunc>
	void AssertCanBootLocalNodeWithPeers(THandlerFunc handler) {
		// Act:
		TTestContext context(NodeFlag::Regular);

		context.waitForNumActiveReaders(1);
		context.waitForNumActiveWriters(1);
		auto stats = context.stats();

		// Assert:
		EXPECT_EQ(1u, stats.NumActiveReaders);
		EXPECT_EQ(1u, stats.NumActiveWriters);
		handler(context, stats);
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
		ionet::Connect(pPool->service(), options, endpoint, [&](auto connectResult, const auto&) -> void {
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
		/// The connection thread pool.
		std::unique_ptr<thread::IoServiceThreadPool> pPool;

		/// The connection io.
		std::shared_ptr<ionet::PacketIo> pIo;
	};

	/// Creates an external connection to the local node on \a port.
	ExternalConnection CreateExternalConnection(unsigned short port);

	// endregion

	// region counter -> stats adapter

	/// Basic statistics about a local node.
	struct BasicLocalNodeStats {
		/// The number of active packet readers.
		uint64_t NumActiveReaders;

		/// The number of active packet writers.
		uint64_t NumActiveWriters;

		/// The number of scheduled tasks.
		uint64_t NumScheduledTasks;

		/// The number of block elements added to the disruptor.
		uint64_t NumAddedBlockElements;

		/// The number of transaction elements added to the disruptor.
		uint64_t NumAddedTransactionElements;
	};

	/// Statistics about a local p2p node.
	struct PeerLocalNodeStats : public BasicLocalNodeStats {
		/// The number of active broadcast packet writers.
		uint64_t NumActiveBroadcastWriters;

		/// The number of unlocked accounts.
		uint64_t NumUnlockedAccounts;
	};

	/// Returns \c true if \a counters contains a counter with \a name.
	bool HasCounter(const local::LocalNodeCounterValues& counters, const std::string& name);

	/// Returns the value of the counter \a name in \a counters.
	uint64_t GetCounterValue(const local::LocalNodeCounterValues& counters, const std::string& name);

	/// Extracts all basic statistics from \a counters.
	BasicLocalNodeStats CountersToBasicLocalNodeStats(const local::LocalNodeCounterValues& counters);

	/// Extracts all basic and peer statistics from \a counters.
	PeerLocalNodeStats CountersToPeerLocalNodeStats(const local::LocalNodeCounterValues& counters);

	// endregion
}}
