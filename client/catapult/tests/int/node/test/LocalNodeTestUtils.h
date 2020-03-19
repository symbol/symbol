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
#include "catapult/local/server/LocalNode.h"
#include "tests/test/core/ThreadPoolTestUtils.h"
#include "tests/test/local/LocalTestUtils.h"
#include "tests/test/nemesis/NemesisCompatibleConfiguration.h"
#include "tests/test/net/SocketTestUtils.h"
#include "tests/test/nodeps/KeyTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace test {

	/// Gets the local node port.
	inline unsigned short GetLocalNodePort() {
		return GetLocalHostPort();
	}

	/// Gets the local node api port.
	inline unsigned short GetLocalNodeApiPort() {
		return GetLocalNodePort() + 1;
	}

	/// Possible node flags.
	enum class NodeFlag {
		/// Node with a single (self) peer.
		Regular = utils::to_underlying_type(LocalNodeFlags::None),

		/// Node with auto harvesting.
		Auto_Harvest = utils::to_underlying_type(LocalNodeFlags::Should_Auto_Harvest),

		/// Node with custom peers.
		Custom_Peers = 4,

		/// Node with a partner node.
		With_Partner = 8,

		/// Simulated api node.
		Simulated_Api = 16,

		/// Node supporting verifiable receipts.
		Verify_Receipts = 32,

		/// Node supporting cache database storage.
		Cache_Database_Storage = 64,

		/// Node supporting verifiable state.
		Verify_State = 128 | Cache_Database_Storage,

		/// Node that should not be booted implicitly.
		Require_Explicit_Boot = 256,

		/// Node supporting auto sync cleanup.
		Auto_Sync_Cleanup = 512
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

		/// Number of active block elements in the disruptor.
		uint64_t NumActiveBlockElements;

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

	/// Gets the value of the counter with \a name in \a counters.
	uint64_t GetCounterValue(const local::LocalNodeCounterValues& counters, const std::string& name);

	/// Extracts all basic statistics from \a counters.
	BasicLocalNodeStats CountersToBasicLocalNodeStats(const local::LocalNodeCounterValues& counters);

	/// Extracts all basic and peer statistics from \a counters.
	PeerLocalNodeStats CountersToPeerLocalNodeStats(const local::LocalNodeCounterValues& counters);

	// endregion

	// region partner nodes

	/// Creates a local partner node with \a publicKey.
	ionet::Node CreateLocalPartnerNode(const Key& publicKey);

	/// Boots a local partner node around \a config with \a keys and specified \a nodeFlag.
	std::unique_ptr<local::LocalNode> BootLocalPartnerNode(
			config::CatapultConfiguration&& config,
			const config::CatapultKeys& keys,
			NodeFlag nodeFlag);

	/// Prepares catapult configuration (\a config) by  updating setings to be compatible with \a nodeFlag.
	void PrepareCatapultConfiguration(config::CatapultConfiguration& config, NodeFlag nodeFlag);

	/// Prepares catapult configuration (\a config) by adding plugins and extensions (via \a addNodeExtensions)
	/// and updating setings to be compatible with \a nodeFlag.
	template<typename TAddNodeExtensions>
	void PrepareCatapultConfiguration(config::CatapultConfiguration& config, TAddNodeExtensions addNodeExtensions, NodeFlag nodeFlag) {
		PrepareCatapultConfiguration(config, nodeFlag);

		// in order for the nemesis block to be processed, at least the transfer plugin needs to be loaded
		AddNemesisPluginExtensions(const_cast<model::BlockChainConfiguration&>(config.BlockChain));
		addNodeExtensions(const_cast<config::ExtensionsConfiguration&>(config.Extensions));
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
		auto pPool = CreateStartedIoThreadPool(1);
		auto options = CreatePacketSocketOptions();
		auto endpoint = CreateLocalHostNodeEndpoint(port);
		auto clientKeyPair = GenerateKeyPair();
		ionet::Connect(pPool->ioContext(), options, endpoint, [&](auto connectResult, const auto&) {
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
		std::unique_ptr<thread::IoThreadPool> pPool;

		/// Connection io.
		std::shared_ptr<ionet::PacketIo> pIo;
	};

	/// Creates an external connection to the local node on \a port.
	ExternalConnection CreateExternalConnection(unsigned short port);

	// endregion
}}
