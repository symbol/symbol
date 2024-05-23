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

#include "LocalTestUtils.h"
#include "catapult/cache_tx/MemoryUtCache.h"
#include "catapult/chain/UtUpdater.h"
#include "catapult/extensions/PluginUtils.h"
#include "catapult/plugins/PluginLoader.h"
#include "catapult/utils/NetworkTime.h"
#include "tests/test/net/CertificateLocator.h"
#include "tests/test/net/NodeTestUtils.h"
#include "tests/test/nodeps/Nemesis.h"
#include "tests/test/nodeps/TestConstants.h"
#include "tests/test/nodeps/TestNetworkConstants.h"
#include "tests/test/other/MutableCatapultConfiguration.h"

namespace catapult {
namespace test {

    namespace {
        constexpr auto Default_Network_Epoch_Adjustment = utils::TimeSpan::FromMilliseconds(1459468800000);

        void SetConnectionsSubConfiguration(config::NodeConfiguration::ConnectionsSubConfiguration& config)
        {
            config.MaxConnections = 25;
            config.MaxConnectionAge = 10;
            config.MaxConnectionBanAge = 100;
            config.NumConsecutiveFailuresBeforeBanning = 100;
        }

        config::NodeConfiguration CreateNodeConfiguration()
        {
            auto config = config::NodeConfiguration::Uninitialized();
            config.Port = GetLocalHostPort();
            config.MaxIncomingConnectionsPerIdentity = 2;

            config.EnableAddressReuse = true;

            config.FileDatabaseBatchSize = File_Database_Batch_Size;

            config.MaxHashesPerSyncAttempt = 4 * 100;
            config.MaxBlocksPerSyncAttempt = 2 * 100;
            config.MaxChainBytesPerSyncAttempt = utils::FileSize::FromKilobytes(8 * 512);

            config.ShortLivedCacheMaxSize = 10;

            // allow transactions with current time to be included in block two
            config.MaxTimeBehindPullTransactionsStart = utils::TimeSpan::FromMilliseconds((CreateDefaultNetworkTimeSupplier()() + utils::TimeSpan::FromHours(1)).unwrap());
            config.UnconfirmedTransactionsCacheMaxSize = utils::FileSize::FromMegabytes(5);

            config.ConnectTimeout = utils::TimeSpan::FromSeconds(10);
            config.SyncTimeout = utils::TimeSpan::FromSeconds(10);

            config.SocketWorkingBufferSize = utils::FileSize::FromKilobytes(4);
            config.MaxPacketDataSize = utils::FileSize::FromMegabytes(100);

            config.BlockDisruptorSlotCount = 4 * 1024;
            config.BlockDisruptorMaxMemorySize = utils::FileSize::FromMegabytes(100);

            config.TransactionDisruptorSlotCount = 16 * 1024;
            config.TransactionDisruptorMaxMemorySize = utils::FileSize::FromMegabytes(100);

            config.MaxTrackedNodes = 5'000;

            config.ListenInterface = "0.0.0.0";

            config.CacheDatabase.MaxWriteBatchSize = utils::FileSize::FromMegabytes(5);

            config.Local.Host = "127.0.0.1";
            config.Local.FriendlyName = "LOCAL";
            config.Local.Roles = ionet::NodeRoles::IPv4 | ionet::NodeRoles::Peer;

            SetConnectionsSubConfiguration(config.OutgoingConnections);

            SetConnectionsSubConfiguration(config.IncomingConnections);
            config.IncomingConnections.BacklogSize = 100;

            config.Banning.DefaultBanDuration = utils::TimeSpan::FromHours(1);
            config.Banning.MaxBanDuration = utils::TimeSpan::FromHours(2);
            config.Banning.KeepAliveDuration = utils::TimeSpan::FromHours(3);
            config.Banning.MaxBannedNodes = 10;
            return config;
        }

        void SetNetwork(model::NetworkInfo& network)
        {
            network.Identifier = model::NetworkIdentifier::Testnet;
            network.NemesisSignerPublicKey = crypto::KeyPair::FromString(Test_Network_Nemesis_Private_Key).publicKey();
            network.GenerationHashSeed = GetNemesisGenerationHashSeed();
            network.EpochAdjustment = Default_Network_Epoch_Adjustment;
        }
    }

    supplier<Timestamp> CreateDefaultNetworkTimeSupplier()
    {
        return []() { return utils::NetworkTime(Default_Network_Epoch_Adjustment).now(); };
    }

    model::BlockchainConfiguration CreatePrototypicalBlockchainConfiguration()
    {
        auto config = model::BlockchainConfiguration::Uninitialized();
        SetNetwork(config.Network);

        config.CurrencyMosaicId = Default_Currency_Mosaic_Id;
        config.HarvestingMosaicId = Default_Harvesting_Mosaic_Id;

        config.BlockGenerationTargetTime = utils::TimeSpan::FromSeconds(20);
        config.BlockTimeSmoothingFactor = 0;
        config.MaxTransactionLifetime = utils::TimeSpan::FromHours(1);

        config.ImportanceGrouping = 1;
        config.MaxRollbackBlocks = 0;
        config.MaxDifficultyBlocks = 60;
        config.DefaultDynamicFeeMultiplier = BlockFeeMultiplier(1);

        config.InitialCurrencyAtomicUnits = Amount(8'999'999'998'000'000);
        config.MaxMosaicAtomicUnits = Amount(9'000'000'000'000'000);

        config.TotalChainImportance = Importance(17'000);
        config.MinHarvesterBalance = Amount(500'000);
        config.MaxHarvesterBalance = config.InitialCurrencyAtomicUnits;

        config.VotingSetGrouping = 1;

        config.MaxTransactionsPerBlock = 200'000;
        return config;
    }

    config::CatapultConfiguration CreateUninitializedCatapultConfiguration()
    {
        MutableCatapultConfiguration config;
        config.Blockchain.ImportanceGrouping = 1;
        config.Blockchain.MaxRollbackBlocks = 0;
        return config.ToConst();
    }

    config::CatapultConfiguration CreatePrototypicalCatapultConfiguration()
    {
        return CreatePrototypicalCatapultConfiguration(""); // create the configuration without a valid data directory
    }

    config::CatapultConfiguration CreatePrototypicalCatapultConfiguration(const std::string& dataDirectory)
    {
        return CreatePrototypicalCatapultConfiguration(CreatePrototypicalBlockchainConfiguration(), dataDirectory);
    }

    config::CatapultConfiguration CreatePrototypicalCatapultConfiguration(
        model::BlockchainConfiguration&& blockchainConfig,
        const std::string& dataDirectory)
    {
        MutableCatapultConfiguration config;
        config.Blockchain = std::move(blockchainConfig);
        config.Node = CreateNodeConfiguration();

        config.User.SeedDirectory = (std::filesystem::path(dataDirectory) / "seed").generic_string();
        config.User.DataDirectory = dataDirectory;
        config.User.CertificateDirectory = dataDirectory.empty() ? GetDefaultCertificateDirectory() : (std::filesystem::path(dataDirectory) / "cert").generic_string();
        return config.ToConst();
    }

    std::unique_ptr<cache::MemoryUtCache> CreateUtCache()
    {
        auto cacheOptions = cache::MemoryCacheOptions(utils::FileSize::FromKilobytes(1), utils::FileSize::FromMegabytes(1));
        return std::make_unique<cache::MemoryUtCache>(cacheOptions);
    }

    std::unique_ptr<cache::MemoryUtCacheProxy> CreateUtCacheProxy()
    {
        auto cacheOptions = cache::MemoryCacheOptions(utils::FileSize::FromKilobytes(1), utils::FileSize::FromMegabytes(1));
        return std::make_unique<cache::MemoryUtCacheProxy>(cacheOptions);
    }

    std::shared_ptr<plugins::PluginManager> CreateDefaultPluginManagerWithRealPlugins()
    {
        auto config = model::BlockchainConfiguration::Uninitialized();
        SetNetwork(config.Network);
        config.MaxTransactionLifetime = utils::TimeSpan::FromHours(1);
        config.ImportanceGrouping = 123;
        config.MaxDifficultyBlocks = 123;
        config.TotalChainImportance = Importance(15);
        return CreatePluginManagerWithRealPlugins(config);
    }

    namespace {
        std::shared_ptr<plugins::PluginManager> CreatePluginManager(
            const model::BlockchainConfiguration& config,
            const plugins::StorageConfiguration& storageConfig,
            const config::UserConfiguration& userConfig,
            const config::InflationConfiguration& inflationConfig)
        {
            std::vector<plugins::PluginModule> modules;
            auto pPluginManager = std::make_shared<plugins::PluginManager>(config, storageConfig, userConfig, inflationConfig);
            LoadPluginByName(*pPluginManager, modules, "", "catapult.plugins.coresystem");

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

    std::shared_ptr<plugins::PluginManager> CreatePluginManagerWithRealPlugins(const model::BlockchainConfiguration& config)
    {
        return CreatePluginManager(
            config,
            plugins::StorageConfiguration(),
            config::UserConfiguration::Uninitialized(),
            config::InflationConfiguration::Uninitialized());
    }

    std::shared_ptr<plugins::PluginManager> CreatePluginManagerWithRealPlugins(const config::CatapultConfiguration& config)
    {
        return CreatePluginManager(config.Blockchain, extensions::CreateStorageConfiguration(config), config.User, config.Inflation);
    }
}
}
