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
#include "catapult/extensions/LocalNodeBootstrapper.h"
#include "catapult/local/BasicLocalNode.h"

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
		stats.NumActiveBroadcastWriters = GetCounterValue(counters, "B WRITERS");
		stats.NumUnlockedAccounts = GetCounterValue(counters, "UNLKED ACCTS");
		return stats;
	}

	// endregion

	// region partner nodes

	namespace {
		constexpr const char* Local_Node_Partner_Private_Key = "8473645728B15F007385CE2889D198D26369D2806DCDED4A9B219FD0DE23A505";
	}

	crypto::KeyPair LoadPartnerServerKeyPair() {
		return crypto::KeyPair::FromPrivate(crypto::PrivateKey::FromString(Local_Node_Partner_Private_Key));
	}

	ionet::Node CreateLocalPartnerNode() {
		auto metadata = ionet::NodeMetadata(model::NetworkIdentifier::Zero, "PARTNER");
		metadata.Roles = ionet::NodeRoles::Api | ionet::NodeRoles::Peer;
		return ionet::Node(LoadPartnerServerKeyPair().publicKey(), CreateLocalHostNodeEndpoint(GetLocalHostPort() + 10), metadata);
	}

	std::unique_ptr<local::BootedLocalNode> BootLocalPartnerNode(
			const std::string& dataDirectory,
			const crypto::KeyPair& keyPair,
			NodeFlag nodeFlag) {
		// partner node is a P2P node on offset ports
		auto config = CreateLocalNodeConfiguration(dataDirectory);
		const_cast<uint16_t&>(config.Node.Port) += 10;
		const_cast<uint16_t&>(config.Node.ApiPort) += 10;

		// make additional configuration modifications
		PrepareLocalNodeConfiguration(config, AddSimplePartnerPluginExtensions, nodeFlag);

		const auto& resourcesPath = dataDirectory + "/resources";
		auto pBootstrapper = std::make_unique<extensions::LocalNodeBootstrapper>(std::move(config), resourcesPath, "Partner");
		pBootstrapper->loadExtensions();

		return local::CreateBasicLocalNode(keyPair, std::move(pBootstrapper));
	}

	// endregion

	// region connection tests

	ExternalConnection CreateExternalConnection(unsigned short port) {
		ExternalConnection connection;
		connection.pPool = CreateStartedIoServiceThreadPool(1);

		auto serverKeyPair = LoadServerKeyPair();
		connection.pIo = ConnectToLocalHost(connection.pPool->service(), port, serverKeyPair.publicKey());
		return connection;
	}

	// endregion
}}
