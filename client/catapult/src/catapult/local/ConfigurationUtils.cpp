#include "ConfigurationUtils.h"
#include "catapult/config/NodeConfiguration.h"

namespace catapult { namespace local {

	cache::MemoryCacheOptions GetUtCacheOptions(const config::NodeConfiguration& config) {
		return cache::MemoryCacheOptions(
				config.UnconfirmedTransactionsCacheMaxResponseSize.bytes(),
				config.UnconfirmedTransactionsCacheMaxSize);
	}
}}
