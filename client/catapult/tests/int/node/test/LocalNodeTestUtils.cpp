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

#include "LocalNodeTestUtils.h"
#include "catapult/extensions/ProcessBootstrapper.h"
#include "tests/test/local/NetworkTestUtils.h"

namespace catapult { namespace test {

	// region counter -> stats adapter

	namespace {
		bool HasName(const local::LocalNodeCounterValue& counter, const std::string& name) {
			return name == counter.id().name();
		}

		void CountersToBasicLocalNodeStats(const local::LocalNodeCounterValues& counters, BasicLocalNodeStats& stats) {
			stats.NumActiveReaders = GetCounterValue(counters, "READERS");
			stats.NumActiveWriters = GetCounterValue(counters, "WRITERS");
			stats.NumScheduledTasks = GetCounterValue(counters, "TASKS");
			stats.NumAddedBlockElements = GetCounterValue(counters, "BLK ELEM TOT");
			stats.NumActiveBlockElements = GetCounterValue(counters, "BLK ELEM ACT");
			stats.NumAddedTransactionElements = GetCounterValue(counters, "TX ELEM TOT");
		}
	}

	bool HasCounter(const local::LocalNodeCounterValues& counters, const std::string& name) {
		return std::any_of(counters.cbegin(), counters.cend(), [&name](const auto& counter) { return HasName(counter, name); });
	}

	uint64_t GetCounterValue(const local::LocalNodeCounterValues& counters, const std::string& name) {
		for (const auto& counter : counters) {
			if (HasName(counter, name))
				return counter.value();
		}

		CATAPULT_THROW_INVALID_ARGUMENT_1("could not find counter with name", name);
	}

	BasicLocalNodeStats CountersToBasicLocalNodeStats(const local::LocalNodeCounterValues& counters) {
		BasicLocalNodeStats stats;
		CountersToBasicLocalNodeStats(counters, stats);
		return stats;
	}

	PeerLocalNodeStats CountersToPeerLocalNodeStats(const local::LocalNodeCounterValues& counters) {
		PeerLocalNodeStats stats;
		CountersToBasicLocalNodeStats(counters, stats);
		stats.NumUnlockedAccounts = GetCounterValue(counters, "UNLKED ACCTS");
		return stats;
	}

	// endregion

	// region partner nodes

	ionet::Node CreateLocalPartnerNode(const Key& publicKey) {
		auto endpoint = CreateLocalHostNodeEndpoint(static_cast<unsigned short>(GetLocalHostPort() + 10));
		auto metadata = ionet::NodeMetadata(model::UniqueNetworkFingerprint(), "PARTNER");
		metadata.Roles = ionet::NodeRoles::Api | ionet::NodeRoles::Peer;
		return ionet::Node({ publicKey, "127.0.0.1" }, endpoint, metadata);
	}

	std::unique_ptr<local::LocalNode> BootLocalPartnerNode(
			config::CatapultConfiguration&& config,
			const config::CatapultKeys& keys,
			NodeFlag nodeFlag) {
		// partner node is a P2P node on offset ports
		const_cast<uint16_t&>(config.Node.Port) = static_cast<uint16_t>(config.Node.Port + 10);

		// make additional configuration modifications
		PrepareCatapultConfiguration(config, AddSimplePartnerPluginExtensions, nodeFlag);

		const auto& resourcesPath = config.User.DataDirectory + "/resources";
		auto disposition = extensions::ProcessDisposition::Production;
		auto pBootstrapper = std::make_unique<extensions::ProcessBootstrapper>(std::move(config), resourcesPath, disposition, "Partner");
		pBootstrapper->loadExtensions();

		return local::CreateLocalNode(keys, std::move(pBootstrapper));
	}

	void PrepareCatapultConfiguration(config::CatapultConfiguration& config, NodeFlag nodeFlag) {
		if (HasFlag(NodeFlag::Cache_Database_Storage, nodeFlag))
			const_cast<config::NodeConfiguration&>(config.Node).EnableCacheDatabaseStorage = true;

		if (HasFlag(NodeFlag::Verify_Receipts, nodeFlag))
			const_cast<model::BlockChainConfiguration&>(config.BlockChain).EnableVerifiableReceipts = true;

		if (HasFlag(NodeFlag::Verify_State, nodeFlag))
			const_cast<model::BlockChainConfiguration&>(config.BlockChain).EnableVerifiableState = true;

		if (HasFlag(NodeFlag::Auto_Sync_Cleanup, nodeFlag))
			const_cast<config::NodeConfiguration&>(config.Node).EnableAutoSyncCleanup = true;
	}

	// endregion

	// region connection tests

	ExternalConnection CreateExternalConnection(unsigned short port) {
		ExternalConnection connection;
		connection.pPool = CreateStartedIoThreadPool(1);
		connection.pIo = ConnectToLocalHost(connection.pPool->ioContext(), port);
		return connection;
	}

	// endregion
}}
