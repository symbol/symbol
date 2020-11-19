/**
*** Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
*** Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
*** All rights reserved.
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

#pragma once
#include "catapult/config/CatapultConfiguration.h"
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

		/// Local node should harvest upon startup.
		Should_Auto_Harvest = 2
	};

	/// Creates a default network time supplier for use in local tests.
	supplier<Timestamp> CreateDefaultNetworkTimeSupplier();

	/// Creates a prototypical block chain configuration that is safe to use in local tests.
	model::BlockChainConfiguration CreatePrototypicalBlockChainConfiguration();

	/// Creates an uninitialized catapult configuration.
	config::CatapultConfiguration CreateUninitializedCatapultConfiguration();

	/// Creates a prototypical catapult configuration that is safe to use in local tests.
	config::CatapultConfiguration CreatePrototypicalCatapultConfiguration();

	/// Creates a test catapult configuration with a storage in the specified directory (\a dataDirectory).
	config::CatapultConfiguration CreatePrototypicalCatapultConfiguration(const std::string& dataDirectory);

	/// Creates a test catapult configuration according to the supplied configuration (\a blockChainConfig)
	/// with a storage in the specified directory (\a dataDirectory).
	config::CatapultConfiguration CreatePrototypicalCatapultConfiguration(
			model::BlockChainConfiguration&& blockChainConfig,
			const std::string& dataDirectory);

	/// Creates a default unconfirmed transactions cache.
	std::unique_ptr<cache::MemoryUtCache> CreateUtCache();

	/// Creates a default unconfirmed transactions cache proxy.
	std::unique_ptr<cache::MemoryUtCacheProxy> CreateUtCacheProxy();

	/// Creates a default plugin manager.
	std::shared_ptr<plugins::PluginManager> CreateDefaultPluginManagerWithRealPlugins();

	/// Creates a plugin manager around \a config.
	std::shared_ptr<plugins::PluginManager> CreatePluginManagerWithRealPlugins(const model::BlockChainConfiguration& config);

	/// Creates a plugin manager around \a config.
	/// \note This overload is the only overload that allows state verification.
	std::shared_ptr<plugins::PluginManager> CreatePluginManagerWithRealPlugins(const config::CatapultConfiguration& config);
}}
