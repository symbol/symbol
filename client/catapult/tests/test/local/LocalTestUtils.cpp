#include "LocalTestUtils.h"
#include "catapult/cache/MemoryUtCache.h"
#include "catapult/chain/UnconfirmedTransactionsUpdater.h"
#include "catapult/crypto/KeyUtils.h"
#include "catapult/plugins/PluginLoader.h"
#include "tests/test/net/NodeTestUtils.h"
#include "tests/test/nodeps/MijinConstants.h"

namespace catapult { namespace test {

	namespace {
		constexpr unsigned short Local_Node_Port = Local_Host_Port;
		constexpr unsigned short Local_Node_Api_Port = Local_Node_Port + 1;
		constexpr const char* Local_Node_Private_Key = "4A236D9F894CF0C4FC8C042DB5DB41CCF35118B7B220163E5B4BC1872C1CD618";
		constexpr const char* Local_Node_Public_Key = "75D8BB873DA8F5CCA741435DE76A46AFC2840803EBF080E931195B048D77F88C";

		constexpr bool ShouldAutoHarvest(uint64_t localNodeFlags) {
			return 0 != (local_node_flags::Should_Auto_Harvest & localNodeFlags);
		}

		config::NodeConfiguration CreateNodeConfiguration() {
			auto config = config::NodeConfiguration::Uninitialized();
			config.Port = Local_Node_Port;
			config.ApiPort = Local_Node_Api_Port;
			config.ShouldAllowAddressReuse = true;

			config.MinBlocksPerSyncAttempt = 100;
			config.MaxBlocksPerSyncAttempt = 4 * 100;
			config.MinChainBytesPerSyncAttempt = utils::FileSize::FromKilobytes(512);
			config.MaxChainBytesPerSyncAttempt = utils::FileSize::FromKilobytes(8 * 512);

			config.ShortLivedCacheMaxSize = 10;

			config.ConnectTimeout = utils::TimeSpan::FromSeconds(10);

			config.SocketWorkingBufferSize = utils::FileSize::FromKilobytes(4);
			config.MaxPacketDataSize = utils::FileSize::FromMegabytes(100);

			config.BlockDisruptorSize = 4 * 1024;
			config.TransactionDisruptorSize = 16 * 1024;
			return config;
		}

		void SetNetwork(model::NetworkInfo& network) {
			network.Identifier = model::NetworkIdentifier::Mijin_Test;
			network.PublicKey = crypto::KeyPair::FromString(test::Mijin_Test_Nemesis_Private_Key).publicKey();
			network.GenerationHash = crypto::ParseKey("57F7DA205008026C776CB6AED843393F04CD458E0AA2D9F1D5F31A402072B2D6");
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
		config.MaxRollbackBlocks = 0;
		config.MaxDifficultyBlocks = 60;

		config.TotalChainBalance = Amount(8'999'999'998'000'000);
		config.MinHarvesterBalance = Amount(1'000'000'000'000);

		config.BlockPruneInterval = 360;
		return config;
	}

	config::LocalNodeConfiguration LoadLocalNodeConfiguration(uint64_t localNodeFlags, const std::string& dataDirectory) {
		return LoadLocalNodeConfiguration(CreateLocalNodeBlockChainConfiguration(), localNodeFlags, dataDirectory);
	}

	config::LocalNodeConfiguration LoadLocalNodeConfiguration(
			model::BlockChainConfiguration&& blockChainConfiguration,
			uint64_t localNodeFlags,
			const std::string& dataDirectory) {
		auto userConfig = config::UserConfiguration::Uninitialized();
		userConfig.BootKey = Local_Node_Private_Key;
		userConfig.HarvestKey = Local_Node_Private_Key;
		userConfig.IsAutoHarvestingEnabled = ShouldAutoHarvest(localNodeFlags);
		userConfig.MaxUnlockedAccounts = 10;
		userConfig.DataDirectory = dataDirectory;

		auto peers = std::vector<ionet::Node>();
		if (0 == (local_node_flags::No_Peers & localNodeFlags))
			peers.push_back(CreateLocalHostNode(crypto::ParseKey(Local_Node_Public_Key), Local_Node_Port));

		return config::LocalNodeConfiguration(
			std::move(blockChainConfiguration),
			CreateNodeConfiguration(),
			config::LoggingConfiguration::Uninitialized(),
			std::move(userConfig),
			std::move(peers));
	}

	config::LocalNodeConfiguration CreatePrototypicalLocalNodeConfiguration() {
		return LoadLocalNodeConfiguration(0, ""); // create the configuration without a valid data directory
	}

	config::LocalNodeConfiguration CreateUninitializedLocalNodeConfiguration() {
		auto blockChainConfig = model::BlockChainConfiguration::Uninitialized();
		blockChainConfig.ImportanceGrouping = 1;
		blockChainConfig.MaxRollbackBlocks = 0;

		auto userConfig = config::UserConfiguration::Uninitialized();
		userConfig.BootKey = Local_Node_Private_Key;
		userConfig.IsAutoHarvestingEnabled = false;

		return config::LocalNodeConfiguration(
				std::move(blockChainConfig),
				config::NodeConfiguration::Uninitialized(),
				config::LoggingConfiguration::Uninitialized(),
				std::move(userConfig),
				{ CreateLocalHostNode(crypto::ParseKey(Local_Node_Public_Key), Local_Node_Port) });
	}

	std::unique_ptr<cache::MemoryUtCache> CreateUnconfirmedTransactionsCache() {
		return std::make_unique<cache::MemoryUtCache>(cache::MemoryUtCacheOptions(1024, 1000));
	}

	std::unique_ptr<chain::UnconfirmedTransactionsUpdater> CreateUnconfirmedTransactionsUpdater(
			const cache::CatapultCache& cache,
			cache::UtCache& unconfirmedTransactionsCache) {
		return std::make_unique<chain::UnconfirmedTransactionsUpdater>(
				unconfirmedTransactionsCache,
				cache,
				chain::ExecutionConfiguration(),
				[]() { return Timestamp(111); });
	}

	std::function<cache::MemoryUtCacheView ()> CreateViewProvider(const cache::MemoryUtCache& unconfirmedTransactionsCache) {
		const auto& viewProvider = unconfirmedTransactionsCache;
		return [&viewProvider]() { return viewProvider.view(); };
	}

	std::shared_ptr<plugins::PluginManager> CreateDefaultPluginManager() {
		auto config = model::BlockChainConfiguration::Uninitialized();
		SetNetwork(config.Network);
		config.TotalChainBalance = Amount(15'000'000);
		config.ImportanceGrouping = 123;
		config.MaxDifficultyBlocks = 123;
		config.BlockPruneInterval = 360;
		return CreateDefaultPluginManager(config);
	}

	std::shared_ptr<plugins::PluginManager> CreateDefaultPluginManager(const model::BlockChainConfiguration& config) {
		std::vector<plugins::PluginModule> modules;
		auto pPluginManager = std::make_shared<plugins::PluginManager>(config);
		for (const auto& pluginName : { "coresystem", "blockdifficultycache" })
			LoadPluginByName(*pPluginManager, modules, "", std::string("catapult.plugins.") + pluginName);

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
