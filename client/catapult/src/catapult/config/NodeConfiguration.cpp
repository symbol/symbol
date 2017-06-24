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

		LOAD_NODE_PROPERTY(MinBlocksPerSyncAttempt);
		LOAD_NODE_PROPERTY(MaxBlocksPerSyncAttempt);
		LOAD_NODE_PROPERTY(MinChainBytesPerSyncAttempt);
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
		LOAD_NODE_PROPERTY(MaxPacketDataSize);

		LOAD_NODE_PROPERTY(BlockDisruptorSize);
		LOAD_NODE_PROPERTY(BlockElementTraceInterval);
		LOAD_NODE_PROPERTY(TransactionDisruptorSize);
		LOAD_NODE_PROPERTY(TransactionElementTraceInterval);
		LOAD_NODE_PROPERTY(TransactionBatchPeriod);

#undef LOAD_NODE_PROPERTY

		utils::VerifyBagSizeLte(bag, 22);
		return config;
	}

#undef LOAD_PROPERTY
}}
