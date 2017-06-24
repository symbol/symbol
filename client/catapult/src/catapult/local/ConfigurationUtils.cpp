#include "ConfigurationUtils.h"
#include "catapult/config/NodeConfiguration.h"

namespace catapult { namespace local {

	cache::MemoryUtCacheOptions GetUnconfirmedTransactionsCacheOptions(const config::NodeConfiguration& config) {
		return cache::MemoryUtCacheOptions(
				config.UnconfirmedTransactionsCacheMaxResponseSize.bytes(),
				config.UnconfirmedTransactionsCacheMaxSize);
	}
}}
