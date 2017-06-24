#pragma once
#include "catapult/config/LocalNodeConfiguration.h"
#include "catapult/crypto/KeyPair.h"
#include "catapult/plugins/PluginManager.h"
#include "tests/test/core/AddressTestUtils.h"
#include "tests/test/net/NodeTestUtils.h"
#include "tests/TestHarness.h"
#include <string>

namespace catapult {
	namespace cache {
		class CatapultCache;
		class MemoryUtCache;
		class MemoryUtCacheView;
		class UtCache;
	}
	namespace chain { class UnconfirmedTransactionsUpdater; }
}

namespace catapult { namespace test {

	namespace local_node_flags {
		/// No special configuration flag is set.
		constexpr uint64_t None = 0;

		/// The local node should harvest upon startup.
		constexpr uint64_t Should_Auto_Harvest = 2;

		/// The local node should not have any peers.
		constexpr uint64_t No_Peers = 4;
	}

	/// Returns server key pair.
	crypto::KeyPair LoadServerKeyPair();

	/// Creates a block chain configuration.
	model::BlockChainConfiguration CreateLocalNodeBlockChainConfiguration();

	/// Creates a test configuration for a local node according to the supplied flags (\a localNodeFlags)
	/// with a storage in the specified directory (\a dataDirectory).
	config::LocalNodeConfiguration LoadLocalNodeConfiguration(uint64_t localNodeFlags, const std::string& dataDirectory);

	/// Creates a test configuration for a local node according to the supplied configuration (\a config) and
	/// flags (\a localNodeFlags) with a storage in the specified directory (\a dataDirectory).
	config::LocalNodeConfiguration LoadLocalNodeConfiguration(
			model::BlockChainConfiguration&& config,
			uint64_t localNodeFlags,
			const std::string& dataDirectory);

	/// Creates a prototypical local node configuration that is safe to use in local node tests.
	config::LocalNodeConfiguration CreatePrototypicalLocalNodeConfiguration();

	/// Creates an uninitialized local node configuration.
	config::LocalNodeConfiguration CreateUninitializedLocalNodeConfiguration();

	/// Creates a default unconfirmed transactions cache.
	std::unique_ptr<cache::MemoryUtCache> CreateUnconfirmedTransactionsCache();

	/// Creates a default unconfirmed transactions updater around \a cache and \a unconfirmedTransactionsCache.
	std::unique_ptr<chain::UnconfirmedTransactionsUpdater> CreateUnconfirmedTransactionsUpdater(
			const cache::CatapultCache& cache,
			cache::UtCache& unconfirmedTransactionsCache);

	/// Creates a default unconfirmed transactions cache view provider around \a unconfirmedTransactionsCache.
	std::function<cache::MemoryUtCacheView ()> CreateViewProvider(const cache::MemoryUtCache& unconfirmedTransactionsCache);

	/// Creates a default plugin manager.
	std::shared_ptr<plugins::PluginManager> CreateDefaultPluginManager();

	/// Creates a default plugin manager around \a config.
	std::shared_ptr<plugins::PluginManager> CreateDefaultPluginManager(const model::BlockChainConfiguration& config);
}}
