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
		class MemoryUtCacheProxy;
		class MemoryUtCacheView;
		class UtCache;
	}
	namespace chain { class UtUpdater; }
}

namespace catapult { namespace test {

	/// Bit flags for configuring a LocalNode under test.
	enum class LocalNodeFlags {
		/// No special configuration flag is set.
		None = 0,

		/// The local node should harvest upon startup.
		Should_Auto_Harvest = 2,
	};

	/// Returns server key pair.
	crypto::KeyPair LoadServerKeyPair();

	/// Creates a block chain configuration.
	model::BlockChainConfiguration CreateLocalNodeBlockChainConfiguration();

	/// Creates a test configuration for a local node with a storage in the specified directory (\a dataDirectory).
	config::LocalNodeConfiguration LoadLocalNodeConfiguration(const std::string& dataDirectory);

	/// Creates a test configuration for a local node according to the supplied configuration (\a config)
	/// with a storage in the specified directory (\a dataDirectory).
	config::LocalNodeConfiguration LoadLocalNodeConfiguration(model::BlockChainConfiguration&& config, const std::string& dataDirectory);

	/// Creates a prototypical local node configuration that is safe to use in local node tests.
	config::LocalNodeConfiguration CreatePrototypicalLocalNodeConfiguration();

	/// Creates an uninitialized local node configuration.
	config::LocalNodeConfiguration CreateUninitializedLocalNodeConfiguration();

	/// Creates a default unconfirmed transactions cache.
	std::unique_ptr<cache::MemoryUtCache> CreateUtCache();

	/// Creates a default unconfirmed transactions cache proxy.
	std::unique_ptr<cache::MemoryUtCacheProxy> CreateUtCacheProxy();

	/// Creates a default plugin manager.
	std::shared_ptr<plugins::PluginManager> CreateDefaultPluginManager();

	/// Creates a default plugin manager around \a config.
	std::shared_ptr<plugins::PluginManager> CreateDefaultPluginManager(const model::BlockChainConfiguration& config);
}}
