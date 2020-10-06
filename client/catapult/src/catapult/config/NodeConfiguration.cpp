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

#include "NodeConfiguration.h"
#include "catapult/utils/ConfigurationBag.h"
#include "catapult/utils/ConfigurationUtils.h"

namespace catapult { namespace config {

#define LOAD_PROPERTY(SECTION, NAME) utils::LoadIniProperty(bag, SECTION, #NAME, config.NAME)

	NodeConfiguration NodeConfiguration::Uninitialized() {
		return NodeConfiguration();
	}

	NodeConfiguration NodeConfiguration::LoadFromBag(const utils::ConfigurationBag& bag) {
		NodeConfiguration config;

#define LOAD_NODE_PROPERTY(NAME) LOAD_PROPERTY("node", NAME)

		LOAD_NODE_PROPERTY(Port);
		LOAD_NODE_PROPERTY(MaxIncomingConnectionsPerIdentity);

		LOAD_NODE_PROPERTY(EnableAddressReuse);
		LOAD_NODE_PROPERTY(EnableSingleThreadPool);
		LOAD_NODE_PROPERTY(EnableCacheDatabaseStorage);
		LOAD_NODE_PROPERTY(EnableAutoSyncCleanup);

		LOAD_NODE_PROPERTY(EnableTransactionSpamThrottling);
		LOAD_NODE_PROPERTY(TransactionSpamThrottlingMaxBoostFee);

		LOAD_NODE_PROPERTY(MaxHashesPerSyncAttempt);
		LOAD_NODE_PROPERTY(MaxBlocksPerSyncAttempt);
		LOAD_NODE_PROPERTY(MaxChainBytesPerSyncAttempt);

		LOAD_NODE_PROPERTY(ShortLivedCacheTransactionDuration);
		LOAD_NODE_PROPERTY(ShortLivedCacheBlockDuration);
		LOAD_NODE_PROPERTY(ShortLivedCachePruneInterval);
		LOAD_NODE_PROPERTY(ShortLivedCacheMaxSize);

		LOAD_NODE_PROPERTY(MinFeeMultiplier);
		LOAD_NODE_PROPERTY(TransactionSelectionStrategy);
		LOAD_NODE_PROPERTY(UnconfirmedTransactionsCacheMaxResponseSize);
		LOAD_NODE_PROPERTY(UnconfirmedTransactionsCacheMaxSize);

		LOAD_NODE_PROPERTY(ConnectTimeout);
		LOAD_NODE_PROPERTY(SyncTimeout);

		LOAD_NODE_PROPERTY(SocketWorkingBufferSize);
		LOAD_NODE_PROPERTY(SocketWorkingBufferSensitivity);
		LOAD_NODE_PROPERTY(MaxPacketDataSize);

		LOAD_NODE_PROPERTY(BlockDisruptorSize);
		LOAD_NODE_PROPERTY(BlockElementTraceInterval);
		LOAD_NODE_PROPERTY(TransactionDisruptorSize);
		LOAD_NODE_PROPERTY(TransactionElementTraceInterval);

		LOAD_NODE_PROPERTY(EnableDispatcherAbortWhenFull);
		LOAD_NODE_PROPERTY(EnableDispatcherInputAuditing);

		LOAD_NODE_PROPERTY(MaxCacheDatabaseWriteBatchSize);
		LOAD_NODE_PROPERTY(MaxTrackedNodes);

		LOAD_NODE_PROPERTY(MinPartnerNodeVersion);
		LOAD_NODE_PROPERTY(MaxPartnerNodeVersion);

		LOAD_NODE_PROPERTY(TrustedHosts);
		LOAD_NODE_PROPERTY(LocalNetworks);

#undef LOAD_NODE_PROPERTY

#define LOAD_LOCALNODE_PROPERTY(NAME) utils::LoadIniProperty(bag, "localnode", #NAME, config.Local.NAME)

		LOAD_LOCALNODE_PROPERTY(Host);
		LOAD_LOCALNODE_PROPERTY(FriendlyName);
		LOAD_LOCALNODE_PROPERTY(Version);
		LOAD_LOCALNODE_PROPERTY(Roles);

#undef LOAD_LOCALNODE_PROPERTY

#define LOAD_OUT_CONNECTIONS_PROPERTY(NAME) utils::LoadIniProperty(bag, "outgoing_connections", #NAME, config.OutgoingConnections.NAME)

		LOAD_OUT_CONNECTIONS_PROPERTY(MaxConnections);
		LOAD_OUT_CONNECTIONS_PROPERTY(MaxConnectionAge);
		LOAD_OUT_CONNECTIONS_PROPERTY(MaxConnectionBanAge);
		LOAD_OUT_CONNECTIONS_PROPERTY(NumConsecutiveFailuresBeforeBanning);

#undef LOAD_OUT_CONNECTIONS_PROPERTY

#define LOAD_IN_CONNECTIONS_PROPERTY(NAME) utils::LoadIniProperty(bag, "incoming_connections", #NAME, config.IncomingConnections.NAME)

		LOAD_IN_CONNECTIONS_PROPERTY(MaxConnections);
		LOAD_IN_CONNECTIONS_PROPERTY(MaxConnectionAge);
		LOAD_IN_CONNECTIONS_PROPERTY(BacklogSize);
		LOAD_IN_CONNECTIONS_PROPERTY(MaxConnectionBanAge);
		LOAD_IN_CONNECTIONS_PROPERTY(NumConsecutiveFailuresBeforeBanning);

#undef LOAD_IN_CONNECTIONS_PROPERTY

#define LOAD_BANNING_PROPERTY(NAME) utils::LoadIniProperty(bag, "banning", #NAME, config.Banning.NAME)

		LOAD_BANNING_PROPERTY(DefaultBanDuration);
		LOAD_BANNING_PROPERTY(MaxBanDuration);
		LOAD_BANNING_PROPERTY(KeepAliveDuration);
		LOAD_BANNING_PROPERTY(MaxBannedNodes);

		LOAD_BANNING_PROPERTY(NumReadRateMonitoringBuckets);
		LOAD_BANNING_PROPERTY(ReadRateMonitoringBucketDuration);
		LOAD_BANNING_PROPERTY(MaxReadRateMonitoringTotalSize);

#undef LOAD_BANNING_PROPERTY

		utils::VerifyBagSizeExact(bag, 36 + 4 + 4 + 5 + 7);
		return config;
	}

#undef LOAD_PROPERTY

	// region utils

	bool IsLocalHost(const std::string& host, const std::unordered_set<std::string>& localNetworks) {
		return std::any_of(localNetworks.cbegin(), localNetworks.cend(), [&host](const auto& localNetwork) {
			return host.size() >= localNetwork.size() && 0 == std::memcmp(&localNetwork[0], &host[0], localNetwork.size());
		});
	}

	// endregion
}}
