#pragma once
#include "UnconfirmedTransactionsCacheViewProvider.h"
#include "catapult/local/BootedLocalNode.h"
#include <memory>

namespace catapult {
	namespace cache { class UtCache; }
	namespace config { class LocalNodeConfiguration; }
	namespace crypto { class KeyPair; }
	namespace io { class BlockStorage; }
	namespace local { namespace api { class ChainScoreProvider; } }
	namespace mongo { namespace plugins { class ExternalCacheStorage; } }
	namespace thread { class MultiServicePool; }
}

namespace catapult { namespace local { namespace api {

	/// Creates and boots a local api node around the specified:
	/// - key pair (\a keyPair)
	/// - configuration (\a config)
	/// - multi service pool (\a pMultiServicePool)
	/// - block storage (\a pBlockStorage)
	/// - chain score provider (\a pChainScoreProvider)
	/// - external cache storage (\a pCacheStorage)
	/// - unconfirmed transactions state (composed of read only \a unconfirmedTransactionsCacheViewProvider
	///   and cache \a pUnconfirmedTransactionCache)
	std::unique_ptr<BootedLocalNode> CreateLocalNode(
			const crypto::KeyPair& keyPair,
			config::LocalNodeConfiguration&& config,
			std::unique_ptr<thread::MultiServicePool>&& pMultiServicePool,
			std::unique_ptr<io::BlockStorage>&& pBlockStorage,
			std::shared_ptr<ChainScoreProvider>&& pChainScoreProvider,
			std::unique_ptr<mongo::plugins::ExternalCacheStorage>&& pCacheStorage,
			const UnconfirmedTransactionsCacheViewProvider& unconfirmedTransactionsCacheViewProvider,
			std::unique_ptr<cache::UtCache>&& pUnconfirmedTransactionCache);
}}}
