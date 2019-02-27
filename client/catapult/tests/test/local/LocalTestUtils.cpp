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

#include "LocalTestUtils.h"
#include "catapult/cache/MemoryUtCache.h"
#include "catapult/chain/UtUpdater.h"
#include "catapult/crypto/KeyUtils.h"
#include "catapult/extensions/PluginUtils.h"
#include "catapult/plugins/PluginLoader.h"
#include "tests/test/net/NodeTestUtils.h"
#include "tests/test/nodeps/MijinConstants.h"
#include "tests/test/nodeps/Nemesis.h"
#include "tests/test/nodeps/TestConstants.h"

namespace catapult { namespace test {

	namespace {
		constexpr auto Local_Node_Private_Key = "4A236D9F894CF0C4FC8C042DB5DB41CCF35118B7B220163E5B4BC1872C1CD618";

		void SetConnectionsSubConfiguration(config::NodeConfiguration::ConnectionsSubConfiguration& config) {
			config.MaxConnections = 25;
			config.MaxConnectionAge = 10;
			config.MaxConnectionBanAge = 100;
			config.NumConsecutiveFailuresBeforeBanning = 100;
		}

		config::NodeConfiguration CreateNodeConfiguration() {
			auto config = config::NodeConfiguration::Uninitialized();
			config.Port = GetLocalHostPort();
			config.ApiPort = GetLocalHostPort() + 1;
			config.ShouldAllowAddressReuse = true;

			config.MaxBlocksPerSyncAttempt = 4 * 100;
			config.MaxChainBytesPerSyncAttempt = utils::FileSize::FromKilobytes(8 * 512);

			config.ShortLivedCacheMaxSize = 10;

			config.UnconfirmedTransactionsCacheMaxSize = 100;

			config.ConnectTimeout = utils::TimeSpan::FromSeconds(10);
			config.SyncTimeout = utils::TimeSpan::FromSeconds(10);

			config.SocketWorkingBufferSize = utils::FileSize::FromKilobytes(4);
			config.MaxPacketDataSize = utils::FileSize::FromMegabytes(100);

			config.BlockDisruptorSize = 4 * 1024;
			config.TransactionDisruptorSize = 16 * 1024;

			config.OutgoingSecurityMode = ionet::ConnectionSecurityMode::None;
			config.IncomingSecurityModes = ionet::ConnectionSecurityMode::None;

			config.MaxCacheDatabaseWriteBatchSize = utils::FileSize::FromMegabytes(5);
			config.MaxTrackedNodes = 5'000;

			config.Local.Host = "127.0.0.1";
			config.Local.FriendlyName = "LOCAL";
			config.Local.Roles = ionet::NodeRoles::Peer;

			SetConnectionsSubConfiguration(config.OutgoingConnections);

			SetConnectionsSubConfiguration(config.IncomingConnections);
			config.IncomingConnections.BacklogSize = 100;
			return config;
		}

		void SetNetwork(model::NetworkInfo& network) {
			network.Identifier = model::NetworkIdentifier::Mijin_Test;
			network.PublicKey = crypto::KeyPair::FromString(test::Mijin_Test_Nemesis_Private_Key).publicKey();
			network.GenerationHash = test::GetNemesisGenerationHash();
		}
	}

	crypto::KeyPair LoadServerKeyPair() {
		return crypto::KeyPair::FromPrivate(crypto::PrivateKey::FromString(Local_Node_Private_Key));
	}

	model::BlockChainConfiguration CreateLocalNodeBlockChainConfiguration() {
		auto config = model::BlockChainConfiguration::Uninitialized();
		SetNetwork(config.Network);

		config.CurrencyMosaicId = Default_Currency_Mosaic_Id;
		config.HarvestingMosaicId = Default_Harvesting_Mosaic_Id;

		config.BlockGenerationTargetTime = utils::TimeSpan::FromSeconds(20);
		config.BlockTimeSmoothingFactor = 0;
		config.MaxTransactionLifetime = utils::TimeSpan::FromHours(1);

		config.ImportanceGrouping = 1;
		config.MaxRollbackBlocks = 10;
		config.MaxDifficultyBlocks = 60;

		config.TotalChainImportance = Importance(17'000);
		config.MinHarvesterBalance = Amount(500'000);

		config.BlockPruneInterval = 360;
		config.MaxTransactionsPerBlock = 200'000;
		return config;
	}

	config::LocalNodeConfiguration CreateLocalNodeConfiguration(const std::string& dataDirectory) {
		return CreateLocalNodeConfiguration(CreateLocalNodeBlockChainConfiguration(), dataDirectory);
	}

	config::LocalNodeConfiguration CreateLocalNodeConfiguration(
			model::BlockChainConfiguration&& blockChainConfiguration,
			const std::string& dataDirectory) {
		auto userConfig = config::UserConfiguration::Uninitialized();
		userConfig.BootKey = Local_Node_Private_Key;
		userConfig.DataDirectory = dataDirectory;

		return config::LocalNodeConfiguration(
				std::move(blockChainConfiguration),
				CreateNodeConfiguration(),
				config::LoggingConfiguration::Uninitialized(),
				std::move(userConfig));
	}

	config::LocalNodeConfiguration CreatePrototypicalLocalNodeConfiguration() {
		return CreateLocalNodeConfiguration(""); // create the configuration without a valid data directory
	}

	config::LocalNodeConfiguration CreateUninitializedLocalNodeConfiguration() {
		auto blockChainConfig = model::BlockChainConfiguration::Uninitialized();
		blockChainConfig.ImportanceGrouping = 1;
		blockChainConfig.MaxRollbackBlocks = 0;

		auto userConfig = config::UserConfiguration::Uninitialized();
		userConfig.BootKey = Local_Node_Private_Key;

		return config::LocalNodeConfiguration(
				std::move(blockChainConfig),
				config::NodeConfiguration::Uninitialized(),
				config::LoggingConfiguration::Uninitialized(),
				std::move(userConfig));
	}

	std::unique_ptr<cache::MemoryUtCache> CreateUtCache() {
		return std::make_unique<cache::MemoryUtCache>(cache::MemoryCacheOptions(1024, 1000));
	}

	std::unique_ptr<cache::MemoryUtCacheProxy> CreateUtCacheProxy() {
		return std::make_unique<cache::MemoryUtCacheProxy>(cache::MemoryCacheOptions(1024, 1000));
	}

	std::shared_ptr<plugins::PluginManager> CreateDefaultPluginManager() {
		auto config = model::BlockChainConfiguration::Uninitialized();
		SetNetwork(config.Network);
		config.MaxTransactionLifetime = utils::TimeSpan::FromHours(1);
		config.ImportanceGrouping = 123;
		config.MaxDifficultyBlocks = 123;
		config.TotalChainImportance = Importance(15);
		config.BlockPruneInterval = 360;
		return CreatePluginManager(config);
	}

	namespace {
		std::shared_ptr<plugins::PluginManager> CreatePluginManager(
				const model::BlockChainConfiguration& config,
				const plugins::StorageConfiguration& storageConfig) {
			std::vector<plugins::PluginModule> modules;
			auto pPluginManager = std::make_shared<plugins::PluginManager>(config, storageConfig);
			LoadPluginByName(*pPluginManager, modules, "", "catapult.coresystem");

			for (const auto& pair : config.Plugins)
				LoadPluginByName(*pPluginManager, modules, "", pair.first);

			return std::shared_ptr<plugins::PluginManager>(
					pPluginManager.get(),
					[pPluginManager, modules = std::move(modules)](const auto*) mutable {
						// destroy the modules after the plugin manager
						pPluginManager.reset();
						modules.clear();
					});
		}
	}

	std::shared_ptr<plugins::PluginManager> CreatePluginManager(const model::BlockChainConfiguration& config) {
		return CreatePluginManager(config, plugins::StorageConfiguration());
	}

	std::shared_ptr<plugins::PluginManager> CreatePluginManager(const config::LocalNodeConfiguration& config) {
		return CreatePluginManager(config.BlockChain, extensions::CreateStorageConfiguration(config));
	}
}}
