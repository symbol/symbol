#include "LocalTestUtils.h"
#include "catapult/cache/MemoryUtCache.h"
#include "catapult/chain/UtUpdater.h"
#include "catapult/crypto/KeyUtils.h"
#include "catapult/plugins/PluginLoader.h"
#include "tests/test/net/NodeTestUtils.h"
#include "tests/test/nodeps/MijinConstants.h"
#include "tests/test/nodeps/Nemesis.h"

namespace catapult { namespace test {

	namespace {
		constexpr unsigned short Local_Node_Port = Local_Host_Port;
		constexpr unsigned short Local_Node_Api_Port = Local_Node_Port + 1;
		constexpr const char* Local_Node_Private_Key = "4A236D9F894CF0C4FC8C042DB5DB41CCF35118B7B220163E5B4BC1872C1CD618";

		config::NodeConfiguration CreateNodeConfiguration() {
			auto config = config::NodeConfiguration::Uninitialized();
			config.Port = Local_Node_Port;
			config.ApiPort = Local_Node_Api_Port;
			config.ShouldAllowAddressReuse = true;

			config.MaxBlocksPerSyncAttempt = 4 * 100;
			config.MaxChainBytesPerSyncAttempt = utils::FileSize::FromKilobytes(8 * 512);

			config.ShortLivedCacheMaxSize = 10;

			config.ConnectTimeout = utils::TimeSpan::FromSeconds(10);

			config.SocketWorkingBufferSize = utils::FileSize::FromKilobytes(4);
			config.MaxPacketDataSize = utils::FileSize::FromMegabytes(100);

			config.BlockDisruptorSize = 4 * 1024;
			config.TransactionDisruptorSize = 16 * 1024;

			config.Local.Host = "127.0.0.1";
			config.Local.FriendlyName = "LOCAL";
			config.Local.Roles = ionet::NodeRoles::Peer;

			config.OutgoingConnections.MaxConnections = 25;
			config.OutgoingConnections.MaxConnectionAge = 10;

			config.IncomingConnections.MaxConnections = 25;
			config.IncomingConnections.MaxConnectionAge = 10;
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

		config.BlockGenerationTargetTime = utils::TimeSpan::FromMilliseconds(1'000);
		config.BlockTimeSmoothingFactor = 0;
		config.MaxTransactionLifetime = utils::TimeSpan::FromHours(1);

		config.ImportanceGrouping = 1;
		config.MaxRollbackBlocks = 10;
		config.MaxDifficultyBlocks = 60;

		config.TotalChainBalance = Amount(8'999'999'998'000'000);
		config.MinHarvesterBalance = Amount(1'000'000'000'000);

		config.BlockPruneInterval = 360;
		return config;
	}

	config::LocalNodeConfiguration LoadLocalNodeConfiguration(const std::string& dataDirectory) {
		return LoadLocalNodeConfiguration(CreateLocalNodeBlockChainConfiguration(), dataDirectory);
	}

	config::LocalNodeConfiguration LoadLocalNodeConfiguration(
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
		return LoadLocalNodeConfiguration(""); // create the configuration without a valid data directory
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
		config.TotalChainBalance = Amount(15'000'000);
		config.BlockPruneInterval = 360;
		return CreateDefaultPluginManager(config);
	}

	std::shared_ptr<plugins::PluginManager> CreateDefaultPluginManager(const model::BlockChainConfiguration& config) {
		std::vector<plugins::PluginModule> modules;
		auto pPluginManager = std::make_shared<plugins::PluginManager>(config);
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
}}
