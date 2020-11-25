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

#include "catapult/config/CatapultConfiguration.h"
#include "catapult/config/CatapultKeys.h"
#include "catapult/crypto/OpensslKeyUtils.h"
#include "catapult/model/Address.h"
#include "catapult/utils/HexParser.h"
#include "tests/test/net/CertificateLocator.h"
#include "tests/test/nodeps/Filesystem.h"
#include "tests/test/nodeps/TestConstants.h"
#include "tests/test/other/MutableCatapultConfiguration.h"
#include "tests/TestHarness.h"
#include <filesystem>

namespace catapult { namespace config {

#define TEST_CLASS CatapultConfigurationTests

	// region CatapultConfiguration file io

	namespace {
		const char* Resources_Path = "../resources";
		const char* Config_Filenames[] = {
			"config-extensions-server.properties",
			"config-inflation.properties",
			"config-logging-server.properties",
			"config-network.properties",
			"config-node.properties",
			"config-user.properties"
		};

		void AssertDefaultBlockChainConfiguration(const model::BlockChainConfiguration& config) {
			// Assert:
			EXPECT_EQ(model::NetworkIdentifier::Private_Test, config.Network.Identifier);
			EXPECT_EQ(model::NodeIdentityEqualityStrategy::Host, config.Network.NodeEqualityStrategy);
			EXPECT_EQ(
					utils::ParseByteArray<Key>("C67F465087EF681824805B7E9FF3B2728A4EE847DE044DE5D9FA415F7660B08E"),
					config.Network.NemesisSignerPublicKey);
			EXPECT_EQ(
					utils::ParseByteArray<GenerationHashSeed>("57F7DA205008026C776CB6AED843393F04CD458E0AA2D9F1D5F31A402072B2D6"),
					config.Network.GenerationHashSeed);

			EXPECT_TRUE(config.EnableVerifiableState);
			EXPECT_TRUE(config.EnableVerifiableReceipts);

			EXPECT_EQ(test::Default_Currency_Mosaic_Id, config.CurrencyMosaicId);
			EXPECT_EQ(test::Default_Harvesting_Mosaic_Id, config.HarvestingMosaicId);

			EXPECT_EQ(utils::TimeSpan::FromSeconds(30), config.BlockGenerationTargetTime);
			EXPECT_EQ(3000u, config.BlockTimeSmoothingFactor);

			EXPECT_EQ(39u, config.ImportanceGrouping);
			EXPECT_EQ(5u, config.ImportanceActivityPercentage);
			EXPECT_EQ(40u, config.MaxRollbackBlocks);
			EXPECT_EQ(60u, config.MaxDifficultyBlocks);
			EXPECT_EQ(BlockFeeMultiplier(10'000), config.DefaultDynamicFeeMultiplier);

			EXPECT_EQ(utils::TimeSpan::FromHours(24), config.MaxTransactionLifetime);
			EXPECT_EQ(utils::TimeSpan::FromMilliseconds(500), config.MaxBlockFutureTime);

			EXPECT_EQ(Amount(8'998'999'998'000'000), config.InitialCurrencyAtomicUnits);
			EXPECT_EQ(Amount(9'000'000'000'000'000), config.MaxMosaicAtomicUnits);

			EXPECT_EQ(Importance(15'000'000), config.TotalChainImportance);
			EXPECT_EQ(Amount(500), config.MinHarvesterBalance);
			EXPECT_EQ(Amount(4'000'000), config.MaxHarvesterBalance);
			EXPECT_EQ(Amount(50'000), config.MinVoterBalance);

			EXPECT_EQ(78u, config.VotingSetGrouping);
			EXPECT_EQ(3u, config.MaxVotingKeysPerAccount);
			EXPECT_EQ(72u, config.MinVotingKeyLifetime);
			EXPECT_EQ(72u * 365, config.MaxVotingKeyLifetime);

			EXPECT_EQ(10u, config.HarvestBeneficiaryPercentage);
			EXPECT_EQ(5u, config.HarvestNetworkPercentage);
			EXPECT_EQ(model::StringToAddress("QDBKIOLZD7B3JH3QAZCRJ7WE6ZXO67TKKIG3COY"), config.HarvestNetworkFeeSinkAddress);

			EXPECT_EQ(200'000u, config.MaxTransactionsPerBlock);

			EXPECT_EQ(Height(111), config.ForkHeights.VotingKeyLinkV2);
			EXPECT_EQ(Height(222), config.ForkHeights.ImportanceBlock);
			EXPECT_EQ(Height(333), config.ForkHeights.AccountRestrictionsV2);

			EXPECT_FALSE(config.Plugins.empty());
		}

		void AssertDefaultNodeConfiguration(const NodeConfiguration& config) {
			// Assert:
			EXPECT_EQ(7900u, config.Port);
			EXPECT_EQ(3u, config.MaxIncomingConnectionsPerIdentity);

			EXPECT_FALSE(config.EnableAddressReuse);
			EXPECT_FALSE(config.EnableSingleThreadPool);
			EXPECT_TRUE(config.EnableCacheDatabaseStorage);
			EXPECT_TRUE(config.EnableAutoSyncCleanup);

			EXPECT_TRUE(config.EnableTransactionSpamThrottling);
			EXPECT_EQ(Amount(10'000'000), config.TransactionSpamThrottlingMaxBoostFee);

			EXPECT_EQ(84u, config.MaxHashesPerSyncAttempt);
			EXPECT_EQ(42u, config.MaxBlocksPerSyncAttempt);
			EXPECT_EQ(utils::FileSize::FromMegabytes(100), config.MaxChainBytesPerSyncAttempt);

			EXPECT_EQ(utils::TimeSpan::FromMinutes(10), config.ShortLivedCacheTransactionDuration);
			EXPECT_EQ(utils::TimeSpan::FromMinutes(100), config.ShortLivedCacheBlockDuration);
			EXPECT_EQ(utils::TimeSpan::FromSeconds(90), config.ShortLivedCachePruneInterval);
			EXPECT_EQ(10'000'000u, config.ShortLivedCacheMaxSize);

			EXPECT_EQ(BlockFeeMultiplier(0), config.MinFeeMultiplier);
			EXPECT_EQ(model::TransactionSelectionStrategy::Oldest, config.TransactionSelectionStrategy);
			EXPECT_EQ(utils::FileSize::FromMegabytes(20), config.UnconfirmedTransactionsCacheMaxResponseSize);
			EXPECT_EQ(1'000'000u, config.UnconfirmedTransactionsCacheMaxSize);

			EXPECT_EQ(utils::TimeSpan::FromSeconds(10), config.ConnectTimeout);
			EXPECT_EQ(utils::TimeSpan::FromSeconds(60), config.SyncTimeout);

			EXPECT_EQ(utils::FileSize::FromKilobytes(512), config.SocketWorkingBufferSize);
			EXPECT_EQ(100u, config.SocketWorkingBufferSensitivity);
			EXPECT_EQ(utils::FileSize::FromMegabytes(150), config.MaxPacketDataSize);

			EXPECT_EQ(4096u, config.BlockDisruptorSize);
			EXPECT_EQ(1u, config.BlockElementTraceInterval);
			EXPECT_EQ(16384u, config.TransactionDisruptorSize);
			EXPECT_EQ(10u, config.TransactionElementTraceInterval);

			EXPECT_TRUE(config.EnableDispatcherAbortWhenFull);
			EXPECT_TRUE(config.EnableDispatcherInputAuditing);

			EXPECT_EQ(5'000u, config.MaxTrackedNodes);

			EXPECT_EQ(ionet::GetCurrentServerVersion(), config.MinPartnerNodeVersion);
			EXPECT_EQ(ionet::GetCurrentServerVersion(), config.MaxPartnerNodeVersion);

			EXPECT_TRUE(config.TrustedHosts.empty());
			EXPECT_EQ(std::unordered_set<std::string>({ "127.0.0.1" }), config.LocalNetworks);
			EXPECT_EQ("0.0.0.0", config.ListenInterface);

			EXPECT_FALSE(config.CacheDatabase.EnableStatistics);
			EXPECT_EQ(0u, config.CacheDatabase.MaxOpenFiles);
			EXPECT_EQ(0u, config.CacheDatabase.MaxBackgroundThreads);
			EXPECT_EQ(0u, config.CacheDatabase.MaxSubcompactionThreads);
			EXPECT_EQ(utils::FileSize::FromMegabytes(0), config.CacheDatabase.BlockCacheSize);
			EXPECT_EQ(utils::FileSize::FromMegabytes(0), config.CacheDatabase.MemtableMemoryBudget);

			EXPECT_EQ(utils::FileSize::FromMegabytes(5), config.CacheDatabase.MaxWriteBatchSize);

			EXPECT_EQ("", config.Local.Host);
			EXPECT_EQ("", config.Local.FriendlyName);
			EXPECT_EQ(ionet::GetCurrentServerVersion(), config.Local.Version);
			EXPECT_EQ(ionet::NodeRoles::IPv4 | ionet::NodeRoles::Peer, config.Local.Roles);

			EXPECT_EQ(10u, config.OutgoingConnections.MaxConnections);
			EXPECT_EQ(200u, config.OutgoingConnections.MaxConnectionAge);
			EXPECT_EQ(20u, config.OutgoingConnections.MaxConnectionBanAge);
			EXPECT_EQ(3u, config.OutgoingConnections.NumConsecutiveFailuresBeforeBanning);

			EXPECT_EQ(512u, config.IncomingConnections.MaxConnections);
			EXPECT_EQ(200u, config.IncomingConnections.MaxConnectionAge);
			EXPECT_EQ(20u, config.IncomingConnections.MaxConnectionBanAge);
			EXPECT_EQ(3u, config.IncomingConnections.NumConsecutiveFailuresBeforeBanning);
			EXPECT_EQ(512u, config.IncomingConnections.BacklogSize);

			EXPECT_EQ(utils::TimeSpan::FromHours(12), config.Banning.DefaultBanDuration);
			EXPECT_EQ(utils::TimeSpan::FromHours(72), config.Banning.MaxBanDuration);
			EXPECT_EQ(utils::TimeSpan::FromHours(48), config.Banning.KeepAliveDuration);
			EXPECT_EQ(5'000u, config.Banning.MaxBannedNodes);

			EXPECT_EQ(4u, config.Banning.NumReadRateMonitoringBuckets);
			EXPECT_EQ(utils::TimeSpan::FromSeconds(15), config.Banning.ReadRateMonitoringBucketDuration);
			EXPECT_EQ(utils::FileSize::FromMegabytes(100), config.Banning.MaxReadRateMonitoringTotalSize);
		}

		void AssertDefaultLoggingConfiguration(
				const LoggingConfiguration& config,
				const std::string& expectedLogFilePattern,
				utils::LogLevel expectedFileLogLevel = utils::LogLevel::info) {
			// Assert:
			// - console (basic)
			EXPECT_EQ(utils::LogSinkType::Sync, config.Console.SinkType);
			EXPECT_EQ(utils::LogLevel::info, config.Console.Level);
			EXPECT_TRUE(config.Console.ComponentLevels.empty());

			// - console (specific)
			EXPECT_EQ(utils::LogColorMode::Ansi, config.Console.ColorMode);

			// - file (basic)
			EXPECT_EQ(utils::LogSinkType::Async, config.File.SinkType);
			EXPECT_EQ(expectedFileLogLevel, config.File.Level);
			EXPECT_TRUE(config.File.ComponentLevels.empty());

			// - file (specific)
			EXPECT_EQ("logs", config.File.Directory);
			EXPECT_EQ(expectedLogFilePattern, config.File.FilePattern);
			EXPECT_EQ(utils::FileSize::FromMegabytes(25), config.File.RotationSize);
			EXPECT_EQ(utils::FileSize::FromMegabytes(2500), config.File.MaxTotalSize);
			EXPECT_EQ(utils::FileSize::FromMegabytes(100), config.File.MinFreeSpace);
		}

		void AssertDefaultUserConfiguration(const UserConfiguration& config) {
			// Assert:
			EXPECT_TRUE(config.EnableDelegatedHarvestersAutoDetection);

			EXPECT_EQ("../data", config.DataDirectory);
			EXPECT_EQ("../certificate", config.CertificateDirectory);
			EXPECT_EQ("../votingkeys", config.VotingKeysDirectory);
			EXPECT_EQ(".", config.PluginsDirectory);
		}

		void AssertDefaultExtensionsConfiguration(
				const ExtensionsConfiguration& config,
				const std::vector<std::string>& expectedExtensions) {
			EXPECT_EQ(expectedExtensions, config.Names);
		}

		void AssertDefaultInflationConfiguration(const InflationConfiguration& config) {
			// Assert:
			EXPECT_EQ(2u, config.InflationCalculator.size());
			EXPECT_TRUE(config.InflationCalculator.contains(Height(1), Amount(100)));
			EXPECT_TRUE(config.InflationCalculator.contains(Height(10000), Amount()));
		}
	}

	TEST(TEST_CLASS, CannotLoadConfigWhenAnyConfigFileIsMissing) {
		// Arrange:
		for (const auto& filenameToRemove : Config_Filenames) {
			// - copy all files into a temp directory
			test::TempDirectoryGuard tempDir;
			for (const auto& configFilename : Config_Filenames) {
				std::filesystem::create_directories(tempDir.name());
				std::filesystem::copy_file(
						std::filesystem::path(Resources_Path) / configFilename,
						std::filesystem::path(tempDir.name()) / configFilename);
			}

			// - remove a file
			CATAPULT_LOG(debug) << "removing " << filenameToRemove;
			EXPECT_TRUE(std::filesystem::remove(std::filesystem::path(tempDir.name()) / filenameToRemove));

			// Act + Assert: attempt to load the config
			EXPECT_THROW(CatapultConfiguration::LoadFromPath(tempDir.name(), "server"), catapult_runtime_error);
		}
	}

	TEST(TEST_CLASS, ResourcesDirectoryContainsAllConfigFiles) {
		// Arrange:
		auto resourcesPath = std::filesystem::path(Resources_Path);
		std::set<std::filesystem::path> expectedFilenames;
		for (const auto& configFilename : Config_Filenames)
			expectedFilenames.insert(resourcesPath / configFilename);

		// Act: collect filenames
		auto numFiles = 0u;
		std::set<std::filesystem::path> actualFilenames;
		for (const auto& directoryEntry : std::filesystem::directory_iterator(resourcesPath)) {
			CATAPULT_LOG(debug) << "found " << directoryEntry.path();
			actualFilenames.insert(directoryEntry.path());
			++numFiles;
		}

		// Assert:
		EXPECT_LE(CountOf(Config_Filenames), numFiles);
		for (const auto& expectedFilename : expectedFilenames)
			EXPECT_CONTAINS(actualFilenames, expectedFilename);
	}

	TEST(TEST_CLASS, CanLoadConfigFromResourcesDirectoryWithServerExtensions) {
		// Act: attempt to load from the "real" resources directory
		auto config = CatapultConfiguration::LoadFromPath(Resources_Path, "server");

		// Assert:
		AssertDefaultBlockChainConfiguration(config.BlockChain);
		AssertDefaultNodeConfiguration(config.Node);
		AssertDefaultLoggingConfiguration(config.Logging, "catapult_server%4N.log");
		AssertDefaultUserConfiguration(config.User);
		AssertDefaultExtensionsConfiguration(config.Extensions, {
			"extension.harvesting", "extension.syncsource",
			"extension.diagnostics", "extension.finalization", "extension.hashcache", "extension.networkheight",
			"extension.nodediscovery", "extension.packetserver", "extension.pluginhandlers", "extension.sync",
			"extension.timesync", "extension.transactionsink", "extension.unbondedpruning"
		});
		AssertDefaultInflationConfiguration(config.Inflation);
	}

	TEST(TEST_CLASS, CanLoadConfigFromResourcesDirectoryWithBrokerExtensions) {
		// Act: attempt to load from the "real" resources directory
		auto config = CatapultConfiguration::LoadFromPath(Resources_Path, "broker");

		// Assert:
		AssertDefaultBlockChainConfiguration(config.BlockChain);
		AssertDefaultNodeConfiguration(config.Node);
		AssertDefaultLoggingConfiguration(config.Logging, "catapult_broker%4N.log");
		AssertDefaultUserConfiguration(config.User);
		AssertDefaultExtensionsConfiguration(config.Extensions, {
			"extension.addressextraction", "extension.mongo", "extension.zeromq",
			"extension.hashcache"
		});
		AssertDefaultInflationConfiguration(config.Inflation);
	}

	TEST(TEST_CLASS, CanLoadConfigFromResourcesDirectoryWithRecoveryExtensions) {
		// Act: attempt to load from the "real" resources directory
		auto config = CatapultConfiguration::LoadFromPath(Resources_Path, "recovery");

		// Assert:
		AssertDefaultBlockChainConfiguration(config.BlockChain);
		AssertDefaultNodeConfiguration(config.Node);
		AssertDefaultLoggingConfiguration(config.Logging, "catapult_recovery%4N.log", utils::LogLevel::debug);
		AssertDefaultUserConfiguration(config.User);
		AssertDefaultExtensionsConfiguration(config.Extensions, { "extension.hashcache" });
		AssertDefaultInflationConfiguration(config.Inflation);
	}

	// endregion

	// region ToLocalNode

	namespace {
		constexpr auto Generation_Hash_Seed_String = "272C4ECC55B7A42A07478A9550543C62673D1599A8362CC662E019049B76B7F2";

		auto CreateCatapultConfiguration() {
			test::MutableCatapultConfiguration config;
			config.BlockChain.Network.Identifier = model::NetworkIdentifier::Private_Test;
			config.BlockChain.Network.GenerationHashSeed = utils::ParseByteArray<GenerationHashSeed>(Generation_Hash_Seed_String);

			config.Node.Port = 9876;
			config.Node.Local.Host = "alice.com";
			config.Node.Local.FriendlyName = "a GREAT node";
			config.Node.Local.Version = ionet::NodeVersion(123);
			config.Node.Local.Roles = ionet::NodeRoles::Api;

			config.User.CertificateDirectory = test::GetDefaultCertificateDirectory();
			return config.ToConst();
		}
	}

	TEST(TEST_CLASS, CanExtractLocalNodeFromConfiguration) {
		// Arrange:
		auto config = CreateCatapultConfiguration();
		auto expectedPublicKey = crypto::ReadPublicKeyFromPublicKeyPemFile(GetCaPublicKeyPemFilename(config.User.CertificateDirectory));

		// Act:
		auto node = ToLocalNode(config);

		// Assert:
		const auto& identity = node.identity();
		EXPECT_EQ(expectedPublicKey, identity.PublicKey);
		EXPECT_NE(Key(), identity.PublicKey);
		EXPECT_EQ("_local_", identity.Host);

		const auto& endpoint = node.endpoint();
		EXPECT_EQ("alice.com", endpoint.Host);
		EXPECT_EQ(9876u, endpoint.Port);

		const auto& metadata = node.metadata();
		EXPECT_EQ(model::NetworkIdentifier::Private_Test, metadata.NetworkFingerprint.Identifier);
		EXPECT_EQ(utils::ParseByteArray<GenerationHashSeed>(Generation_Hash_Seed_String), metadata.NetworkFingerprint.GenerationHashSeed);
		EXPECT_EQ("a GREAT node", metadata.Name);
		EXPECT_EQ(ionet::NodeVersion(123), metadata.Version);
		EXPECT_EQ(ionet::NodeRoles::Api, metadata.Roles);
	}

	// endregion
}}
