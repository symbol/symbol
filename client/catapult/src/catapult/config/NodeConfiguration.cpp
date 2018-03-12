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
		LOAD_NODE_PROPERTY(ApiPort);
		LOAD_NODE_PROPERTY(ShouldAllowAddressReuse);
		LOAD_NODE_PROPERTY(ShouldUseSingleThreadPool);

		LOAD_NODE_PROPERTY(MaxBlocksPerSyncAttempt);
		LOAD_NODE_PROPERTY(MaxChainBytesPerSyncAttempt);

		LOAD_NODE_PROPERTY(ShortLivedCacheTransactionDuration);
		LOAD_NODE_PROPERTY(ShortLivedCacheBlockDuration);
		LOAD_NODE_PROPERTY(ShortLivedCachePruneInterval);
		LOAD_NODE_PROPERTY(ShortLivedCacheMaxSize);

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

		LOAD_NODE_PROPERTY(ShouldAbortWhenDispatcherIsFull);
		LOAD_NODE_PROPERTY(ShouldAuditDispatcherInputs);
		LOAD_NODE_PROPERTY(ShouldPrecomputeTransactionAddresses);

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

#undef LOAD_OUT_CONNECTIONS_PROPERTY

#define LOAD_IN_CONNECTIONS_PROPERTY(NAME) utils::LoadIniProperty(bag, "incoming_connections", #NAME, config.IncomingConnections.NAME)

		LOAD_IN_CONNECTIONS_PROPERTY(MaxConnections);
		LOAD_IN_CONNECTIONS_PROPERTY(MaxConnectionAge);
		LOAD_IN_CONNECTIONS_PROPERTY(BacklogSize);

#undef LOAD_IN_CONNECTIONS_PROPERTY

		auto extensionsPair = utils::ExtractSectionAsUnorderedSet(bag, "extensions");
		config.Extensions = extensionsPair.first;

		utils::VerifyBagSizeLte(bag, 24 + 4 + 2 + 3 + extensionsPair.second);
		return config;
	}

#undef LOAD_PROPERTY
}}
