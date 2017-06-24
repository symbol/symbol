#include "catapult/config/LocalNodeConfiguration.h"
#include "catapult/crypto/KeyUtils.h"
#include "tests/test/nodeps/Filesystem.h"
#include "tests/TestHarness.h"
#include <boost/filesystem.hpp>

namespace catapult { namespace config {

	// region LocalNodeConfiguration file io

	namespace {
		const char* Resources_Path = "../resources";
		const char* Config_Filenames[] = {
			"config-log.properties",
			"config-network.properties",
			"config-node.properties",
			"config-user.properties",
			"peers-mijin.json"
		};

		void AssertDefaultBlockChainConfiguration(const model::BlockChainConfiguration& config) {
			// Assert:
			EXPECT_EQ(model::NetworkIdentifier::Mijin_Test, config.Network.Identifier);
			EXPECT_EQ(crypto::ParseKey("B4F12E7C9F6946091E2CB8B6D3A12B50D17CCBBF646386EA27CE2946A7423DCF"), config.Network.PublicKey);
			EXPECT_EQ(crypto::ParseKey("57F7DA205008026C776CB6AED843393F04CD458E0AA2D9F1D5F31A402072B2D6"), config.Network.GenerationHash);

			EXPECT_EQ(utils::TimeSpan::FromSeconds(15), config.BlockGenerationTargetTime);
			EXPECT_EQ(3000u, config.BlockTimeSmoothingFactor);

			EXPECT_EQ(359u, config.ImportanceGrouping);
			EXPECT_EQ(360u, config.MaxRollbackBlocks);
			EXPECT_EQ(60u, config.MaxDifficultyBlocks);

			EXPECT_EQ(utils::TimeSpan::FromHours(24), config.MaxTransactionLifetime);
			EXPECT_EQ(utils::TimeSpan::FromSeconds(10), config.MaxBlockFutureTime);

			EXPECT_EQ(Amount(8'999'999'998'000'000), config.TotalChainBalance);
			EXPECT_EQ(Amount(1'000'000'000'000), config.MinHarvesterBalance);

			EXPECT_EQ(360u, config.BlockPruneInterval);
			EXPECT_EQ(15'000u, config.MaxTransactionsPerBlock);

			EXPECT_FALSE(config.Plugins.empty());
		}

		void AssertDefaultNodeConfiguration(const NodeConfiguration& config) {
			// Assert:
			EXPECT_EQ(7900u, config.Port);
			EXPECT_EQ(7901u, config.ApiPort);
			EXPECT_FALSE(config.ShouldAllowAddressReuse);

			EXPECT_EQ(100u, config.MinBlocksPerSyncAttempt);
			EXPECT_EQ(400u, config.MaxBlocksPerSyncAttempt);
			EXPECT_EQ(utils::FileSize::FromKilobytes(512), config.MinChainBytesPerSyncAttempt);
			EXPECT_EQ(utils::FileSize::FromMegabytes(10), config.MaxChainBytesPerSyncAttempt);

			EXPECT_EQ(utils::TimeSpan::FromMinutes(10), config.ShortLivedCacheTransactionDuration);
			EXPECT_EQ(utils::TimeSpan::FromMinutes(100), config.ShortLivedCacheBlockDuration);
			EXPECT_EQ(utils::TimeSpan::FromSeconds(90), config.ShortLivedCachePruneInterval);
			EXPECT_EQ(1'000'000u, config.ShortLivedCacheMaxSize);

			EXPECT_EQ(utils::FileSize::FromMegabytes(20), config.UnconfirmedTransactionsCacheMaxResponseSize);
			EXPECT_EQ(1'000'000u, config.UnconfirmedTransactionsCacheMaxSize);

			EXPECT_EQ(utils::TimeSpan::FromSeconds(10), config.ConnectTimeout);
			EXPECT_EQ(utils::TimeSpan::FromSeconds(60), config.SyncTimeout);

			EXPECT_EQ(utils::FileSize::FromKilobytes(512), config.SocketWorkingBufferSize);
			EXPECT_EQ(utils::FileSize::FromMegabytes(150), config.MaxPacketDataSize);

			EXPECT_EQ(4096u, config.BlockDisruptorSize);
			EXPECT_EQ(1u, config.BlockElementTraceInterval);
			EXPECT_EQ(16384u, config.TransactionDisruptorSize);
			EXPECT_EQ(10u, config.TransactionElementTraceInterval);
			EXPECT_EQ(utils::TimeSpan::FromMilliseconds(500), config.TransactionBatchPeriod);
		}

		void AssertDefaultLoggingConfiguration(const LoggingConfiguration& config) {
			// Assert:
			// - console (basic)
			EXPECT_EQ(utils::LogSinkType::Sync, config.Console.SinkType);
			EXPECT_EQ(utils::LogLevel::Info, config.Console.Level);
			EXPECT_TRUE(config.Console.ComponentLevels.empty());

			// - console (specific)
			EXPECT_EQ(utils::LogColorMode::Ansi, config.Console.ColorMode);

			// - file (basic)
			EXPECT_EQ(utils::LogSinkType::Async, config.File.SinkType);
			EXPECT_EQ(utils::LogLevel::Info, config.File.Level);
			EXPECT_TRUE(config.File.ComponentLevels.empty());

			// - file (specific)
			EXPECT_EQ("logs", config.File.Directory);
			EXPECT_EQ("catapult_server%4N.log", config.File.FilePattern);
			EXPECT_EQ(utils::FileSize::FromMegabytes(25), config.File.RotationSize);
			EXPECT_EQ(utils::FileSize::FromMegabytes(2500), config.File.MaxTotalSize);
			EXPECT_EQ(utils::FileSize::FromMegabytes(100), config.File.MinFreeSpace);
		}

		void AssertDefaultUserConfiguration(const UserConfiguration& config) {
			// Assert:
			EXPECT_EQ("0000000000000000000000000000000000000000000000000000000000000000", config.BootKey);
			EXPECT_EQ("", config.HarvestKey);
			EXPECT_FALSE(config.IsAutoHarvestingEnabled);
			EXPECT_EQ(5u, config.MaxUnlockedAccounts);

			EXPECT_EQ("../data", config.DataDirectory);
			EXPECT_EQ(".", config.PluginsDirectory);
		}
	}

	TEST(LocalNodeConfigurationTests, CannotLoadConfigWhenAnyConfigFileIsMissing) {
		// Arrange:
		for (const auto& filenameToRemove : Config_Filenames) {
			// - copy all files into a temp directory
			test::TempDirectoryGuard tempDir;
			for (const auto& configFilename : Config_Filenames) {
				boost::filesystem::create_directories(tempDir.name());
				boost::filesystem::copy_file(
						boost::filesystem::path(Resources_Path) / configFilename,
						boost::filesystem::path(tempDir.name()) / configFilename);
			}

			// - remove a file
			CATAPULT_LOG(debug) << "removing " << filenameToRemove;
			EXPECT_TRUE(boost::filesystem::remove(boost::filesystem::path(tempDir.name()) / filenameToRemove));

			// Act: attempt to load the config
			EXPECT_THROW(
					LocalNodeConfiguration::LoadFromPath(tempDir.name()),
					catapult_runtime_error);
		}
	}

	TEST(LocalNodeConfigurationTests, ResourcesDirectoryContainsAllConfigFiles) {
		// Arrange:
		auto resourcesPath = boost::filesystem::path(Resources_Path);
		std::set<boost::filesystem::path> expectedFilenames;
		for (const auto& configFilename : Config_Filenames)
			expectedFilenames.insert(resourcesPath / configFilename);

		// Act: collect filenames
		auto numFiles = 0u;
		std::set<boost::filesystem::path> actualFilenames;
		for (const auto& path : boost::filesystem::directory_iterator(resourcesPath)) {
			CATAPULT_LOG(debug) << "found " << path;
			actualFilenames.insert(path);
			++numFiles;
		}

		// Assert:
		EXPECT_LE(CountOf(Config_Filenames), numFiles);
		for (const auto& expectedFilename : expectedFilenames)
			EXPECT_NE(actualFilenames.cend(), actualFilenames.find(expectedFilename)) << "expected " << expectedFilename;
	}

	TEST(LocalNodeConfigurationTests, CanLoadConfigFromResourcesDirectory) {
		// Act: attempt to load from the "real" resources directory
		auto config = LocalNodeConfiguration::LoadFromPath(Resources_Path);

		// Assert:
		AssertDefaultBlockChainConfiguration(config.BlockChain);
		AssertDefaultNodeConfiguration(config.Node);
		AssertDefaultLoggingConfiguration(config.Logging);
		AssertDefaultUserConfiguration(config.User);
		EXPECT_EQ(4u, config.Peers.size());
	}

	// endregion
}}
