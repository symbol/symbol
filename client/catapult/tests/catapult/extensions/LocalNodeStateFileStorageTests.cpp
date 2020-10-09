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

#include "catapult/extensions/LocalNodeStateFileStorage.h"
#include "catapult/cache/CatapultCache.h"
#include "catapult/cache/SupplementalData.h"
#include "catapult/cache_core/AccountStateCache.h"
#include "catapult/cache_core/AccountStateCacheSubCachePlugin.h"
#include "catapult/cache_core/BlockStatisticCache.h"
#include "catapult/cache_core/BlockStatisticCacheSubCachePlugin.h"
#include "catapult/config/CatapultDataDirectory.h"
#include "catapult/consumers/BlockChainSyncHandlers.h"
#include "catapult/extensions/LocalNodeChainScore.h"
#include "catapult/io/IndexFile.h"
#include "catapult/model/Address.h"
#include "catapult/model/BlockChainConfiguration.h"
#include "tests/test/cache/CacheTestUtils.h"
#include "tests/test/core/AccountStateTestUtils.h"
#include "tests/test/core/StateTestUtils.h"
#include "tests/test/local/LocalNodeTestState.h"
#include "tests/test/local/LocalTestUtils.h"
#include "tests/test/nemesis/NemesisCompatibleConfiguration.h"
#include "tests/test/nodeps/Filesystem.h"
#include "tests/test/plugins/PluginManagerFactory.h"
#include "tests/TestHarness.h"

namespace catapult { namespace extensions {

#define TEST_CLASS LocalNodeStateFileStorageTests

	namespace {
		constexpr auto Default_Network_Id = model::NetworkIdentifier::Private_Test;
		constexpr auto Harvesting_Mosaic_Id = MosaicId(9876);

		constexpr size_t Account_Cache_Size = 123;
		constexpr size_t Block_Cache_Size = 200;

		// region seed utils

		void PopulateAccountStateCache(cache::AccountStateCacheDelta& cacheDelta) {
			for (auto i = 2u; i < Account_Cache_Size + 2; ++i) {
				auto publicKey = test::GenerateRandomByteArray<Key>();
				state::AccountState* pAccountState;
				if (0 == i % 2) {
					cacheDelta.addAccount(publicKey, Height(i / 2));
					pAccountState = &cacheDelta.find(publicKey).get();
				} else {
					auto address = model::PublicKeyToAddress(publicKey, Default_Network_Id);
					cacheDelta.addAccount(address, Height(i / 2));
					pAccountState = &cacheDelta.find(address).get();
				}

				pAccountState->Balances.credit(Harvesting_Mosaic_Id, Amount(10));
				test::RandomFillAccountData(i, *pAccountState);

				cacheDelta.updateHighValueAccounts(Height(1));
			}
		}

		void PopulateBlockStatisticCache(cache::BlockStatisticCacheDelta& cacheDelta) {
			for (auto i = 0u; i < Block_Cache_Size; ++i) {
				auto seed = i + 1;
				cacheDelta.insert({ Height(seed), Timestamp(2 * seed), Difficulty(3 * seed), BlockFeeMultiplier(4 * seed) });
			}
		}

		void RandomSeedCache(cache::CatapultCache& catapultCache, const state::CatapultState& state) {
			// Arrange: seed the cache with random data
			{
				auto delta = catapultCache.createDelta();
				PopulateAccountStateCache(delta.sub<cache::AccountStateCache>());
				PopulateBlockStatisticCache(delta.sub<cache::BlockStatisticCache>());
				delta.dependentState() = state;
				catapultCache.commit(Height(54321));
			}

			// Sanity: data was seeded
			auto view = catapultCache.createView();
			EXPECT_EQ(Account_Cache_Size, view.sub<cache::AccountStateCache>().size());
			EXPECT_EQ(Block_Cache_Size, view.sub<cache::BlockStatisticCache>().size());
		}

		cache::SupplementalData CreateDeterministicSupplementalData() {
			cache::SupplementalData supplementalData;
			supplementalData.ChainScore = model::ChainScore(0x1234567890ABCDEF, 0xFEDCBA0987654321);
			supplementalData.State = test::CreateDeterministicCatapultState();
			return supplementalData;
		}

		void PrepareAndSaveCompleteState(const config::CatapultDirectory& directory, cache::CatapultCache& cache) {
			// Arrange:
			auto supplementalData = CreateDeterministicSupplementalData();
			RandomSeedCache(cache, supplementalData.State);

			LocalNodeStateSerializer serializer(directory);
			serializer.save(cache, supplementalData.ChainScore);
		}

		void PrepareAndSaveSummaryState(const config::CatapultDirectory& directory, cache::CatapultCache& cache) {
			// Arrange:
			auto supplementalData = CreateDeterministicSupplementalData();
			RandomSeedCache(cache, supplementalData.State);

			auto storages = const_cast<const cache::CatapultCache&>(cache).storages();
			LocalNodeStateSerializer serializer(directory);
			serializer.save(cache.createDelta(), storages, supplementalData.ChainScore, Height(54321));
		}

		void AssertPreparedData(const StateHeights& heights, const LocalNodeStateRef& stateRef) {
			// Assert: check heights
			EXPECT_EQ(Height(54321), heights.Cache);
			EXPECT_EQ(Height(1), heights.Storage);

			// - check supplemental data
			EXPECT_EQ(model::ChainScore(0x1234567890ABCDEF, 0xFEDCBA0987654321), stateRef.Score.get());

			auto cacheView = stateRef.Cache.createView();
			test::AssertEqual(CreateDeterministicSupplementalData().State, cacheView.dependentState());
			EXPECT_EQ(Height(54321), cacheView.height());
		}

		// endregion
	}

	// region HasSerializedState

	TEST(TEST_CLASS, HasSerializedStateReturnsTrueWhenSupplementalDataFileExists) {
		// Arrange: create a sentinel file
		test::TempDirectoryGuard tempDir;
		auto stateDirectory = config::CatapultDirectory(tempDir.name() + "/zstate");
		boost::filesystem::create_directories(stateDirectory.path());
		io::IndexFile(stateDirectory.file("supplemental.dat")).set(123);

		// Act:
		auto result = HasSerializedState(stateDirectory);

		// Assert:
		EXPECT_TRUE(result);
	}

	TEST(TEST_CLASS, HasSerializedStateReturnsFalseWhenSupplementalDataFileDoesNotExist) {
		// Arrange:
		test::TempDirectoryGuard tempDir;
		auto stateDirectory = config::CatapultDirectory(tempDir.name() + "/zstate");

		// Act:
		auto result = HasSerializedState(stateDirectory);

		// Assert:
		EXPECT_FALSE(result);
	}

	// endregion

	// region LoadDependentStateFromDirectory

	TEST(TEST_CLASS, LoadDependentStateFromDirectory_LoadsAndUpdatesDependentState) {
		// Arrange: seed and save the cache state with rocks disabled
		test::TempDirectoryGuard tempDir;
		auto stateDirectory = config::CatapultDirectory(tempDir.name() + "/zstate");
		auto blockChainConfig = model::BlockChainConfiguration::Uninitialized();
		auto originalCache = test::CoreSystemCacheFactory::Create(blockChainConfig);

		// - save the state
		PrepareAndSaveCompleteState(stateDirectory, originalCache);

		// Sanity:
		auto cache = test::CoreSystemCacheFactory::Create(blockChainConfig);
		test::AssertEqual(state::CatapultState(), cache.createView().dependentState());

		// Act: load the state
		LoadDependentStateFromDirectory(stateDirectory, cache);

		// Assert: cache was updated
		auto cacheView = cache.createView();
		test::AssertEqual(CreateDeterministicSupplementalData().State, cacheView.dependentState());
		EXPECT_EQ(Height(54321), cacheView.height());
	}

	// endregion

	// region LoadStateFromDirectory - first boot

	TEST(TEST_CLASS, NemesisBlockIsExecutedWhenSupplementalDataFileIsNotPresent) {
		// Arrange: seed and save the cache state (real plugin manager is needed to execute nemesis)
		test::TempDirectoryGuard tempDir;
		config::CatapultDataDirectoryPreparer::Prepare(tempDir.name());

		auto stateDirectory = config::CatapultDirectory(tempDir.name() + "/zstate");
		auto config = test::CreateCatapultConfigurationWithNemesisPluginExtensions(tempDir.name());

		{
			auto pPluginManager = test::CreatePluginManagerWithRealPlugins(config);
			auto originalCache = pPluginManager->createCache();
			PrepareAndSaveCompleteState(stateDirectory, originalCache);
		}

		// - remove the supplemental data file
		ASSERT_TRUE(boost::filesystem::remove(stateDirectory.file("supplemental.dat")));

		// Act: load the state
		auto pPluginManager = test::CreatePluginManagerWithRealPlugins(config);
		test::LocalNodeTestState loadedState(config.BlockChain, stateDirectory.str(), pPluginManager->createCache());
		auto heights = LoadStateFromDirectory(stateDirectory, loadedState.ref(), *pPluginManager);

		// Assert:
		EXPECT_EQ(Height(1), heights.Cache);
		EXPECT_EQ(Height(1), heights.Storage);

		EXPECT_EQ(model::ChainScore(1), loadedState.ref().Score.get());

		auto cacheView = loadedState.ref().Cache.createView();
		auto expectedState = state::CatapultState();
		expectedState.LastRecalculationHeight = model::ImportanceHeight(1);
		expectedState.LastFinalizedHeight = Height(1);
		expectedState.DynamicFeeMultiplier = BlockFeeMultiplier(1);
		expectedState.NumTotalTransactions = 31 + 11;
		test::AssertEqual(expectedState, cacheView.dependentState());
		EXPECT_EQ(Height(1), cacheView.height());
	}

	// endregion

	// region prepare helpers

	namespace {
		void PrepareNonexistentDirectory(const config::CatapultDirectory& directory) {
			// Sanity:
			EXPECT_FALSE(boost::filesystem::exists(directory.path()));
		}

		void PrepareEmptyDirectory(const config::CatapultDirectory& directory) {
			// Arrange:
			boost::filesystem::create_directories(directory.path());

			// Sanity:
			EXPECT_TRUE(boost::filesystem::exists(directory.path()));
		}

		void PrepareDirectoryWithSentinel(const config::CatapultDirectory& directory) {
			// Arrange:
			boost::filesystem::create_directories(directory.path());
			io::IndexFile(directory.file("sentinel")).set(1);

			// Sanity:
			EXPECT_TRUE(boost::filesystem::exists(directory.path()));
			EXPECT_TRUE(boost::filesystem::exists(directory.file("sentinel")));
		}
	}

	// endregion

	// region LoadStateFromDirectory / LocalNodeStateSerializer (CatapultCache)

	namespace {
		template<typename TPrepare>
		void RunSaveAndLoadCompleteStateTest(TPrepare prepare) {
			// Arrange: seed and save the cache state with rocks disabled
			test::TempDirectoryGuard tempDir;
			auto stateDirectory = config::CatapultDirectory(tempDir.name() + "/zstate");
			auto blockChainConfig = model::BlockChainConfiguration::Uninitialized();
			auto originalCache = test::CoreSystemCacheFactory::Create(blockChainConfig);

			// - run additional preparation
			prepare(stateDirectory);

			// Act: save the state
			PrepareAndSaveCompleteState(stateDirectory, originalCache);

			// Act: load the state
			test::LocalNodeTestState loadedState(
					blockChainConfig,
					stateDirectory.str(),
					test::CoreSystemCacheFactory::Create(blockChainConfig));
			auto pluginManager = test::CreatePluginManager();
			auto heights = LoadStateFromDirectory(stateDirectory, loadedState.ref(), pluginManager);

			// Assert:
			AssertPreparedData(heights, loadedState.ref());

			auto expectedView = originalCache.createView();
			auto actualView = loadedState.ref().Cache.createView();
			EXPECT_EQ(expectedView.sub<cache::AccountStateCache>().size(), actualView.sub<cache::AccountStateCache>().size());
			EXPECT_EQ(expectedView.sub<cache::BlockStatisticCache>().size(), actualView.sub<cache::BlockStatisticCache>().size());

			EXPECT_EQ(3u, test::CountFilesAndDirectories(stateDirectory.path()));
			for (const auto* supplementalFilename : { "supplemental.dat", "AccountStateCache.dat", "BlockStatisticCache.dat" })
				EXPECT_TRUE(boost::filesystem::exists(stateDirectory.file(supplementalFilename))) << supplementalFilename;
		}
	}

	TEST(TEST_CLASS, CanSaveAndLoadCompleteState_DirectoryDoesNotExist) {
		RunSaveAndLoadCompleteStateTest(PrepareNonexistentDirectory);
	}

	TEST(TEST_CLASS, CanSaveAndLoadCompleteState_DirectoryExists) {
		RunSaveAndLoadCompleteStateTest(PrepareEmptyDirectory);
	}

	// endregion

	// region LoadStateFromDirectory / LocalNodeStateSerializer (CatapultCacheDelta)

	namespace {
		cache::CatapultCache CreateCacheWithRealCoreSystemPlugins(const std::string& databaseDirectory) {
			auto cacheConfig = cache::CacheConfiguration(databaseDirectory, utils::FileSize(), cache::PatriciaTreeStorageMode::Enabled);

			cache::AccountStateCacheTypes::Options options;
			options.ImportanceGrouping = 1;
			options.MinHarvesterBalance = Amount(1);
			options.HarvestingMosaicId = Harvesting_Mosaic_Id;

			std::vector<std::unique_ptr<cache::SubCachePlugin>> subCaches;
			subCaches.push_back(std::make_unique<cache::AccountStateCacheSubCachePlugin>(cacheConfig, options));
			subCaches.push_back(std::make_unique<cache::BlockStatisticCacheSubCachePlugin>(111));
			return cache::CatapultCache(std::move(subCaches));
		}

		template<typename TPrepare>
		void RunSaveAndLoadSummaryStateTest(TPrepare prepare) {
			// Arrange: seed and save the cache state with rocks enabled
			test::TempDirectoryGuard tempDir;
			auto stateDirectory = config::CatapultDirectory(tempDir.name() + "/zstate");
			auto blockChainConfig = model::BlockChainConfiguration::Uninitialized();
			auto originalCache = CreateCacheWithRealCoreSystemPlugins(tempDir.name() + "/db");

			// - run additional preparation
			prepare(stateDirectory);

			// Act: save the state
			PrepareAndSaveSummaryState(stateDirectory, originalCache);

			// Act: load the state
			test::LocalNodeTestState loadedState(
					blockChainConfig,
					stateDirectory.str(),
					CreateCacheWithRealCoreSystemPlugins(tempDir.name() + "/db2"));
			auto pluginManager = test::CreatePluginManager();
			auto heights = LoadStateFromDirectory(stateDirectory, loadedState.ref(), pluginManager);

			// Assert:
			AssertPreparedData(heights, loadedState.ref());

			auto expectedView = originalCache.createView();
			auto actualView = loadedState.ref().Cache.createView();
			const auto& expectedAccountStateCache = expectedView.sub<cache::AccountStateCache>();
			const auto& actualAccountStateCache = actualView.sub<cache::AccountStateCache>();

			EXPECT_EQ(0u, actualAccountStateCache.size());
			EXPECT_FALSE(actualAccountStateCache.highValueAccounts().addresses().empty());
			EXPECT_EQ(expectedAccountStateCache.highValueAccounts().addresses(), actualAccountStateCache.highValueAccounts().addresses());
			EXPECT_EQ(expectedView.sub<cache::BlockStatisticCache>().size(), actualView.sub<cache::BlockStatisticCache>().size());

			EXPECT_EQ(3u, test::CountFilesAndDirectories(stateDirectory.path()));
			for (const auto* supplementalFilename : { "supplemental.dat", "AccountStateCache_summary.dat", "BlockStatisticCache.dat" })
				EXPECT_TRUE(boost::filesystem::exists(stateDirectory.file(supplementalFilename))) << supplementalFilename;
		}
	}

	TEST(TEST_CLASS, CanSaveAndLoadSummaryState_DirectoryDoesNotExist) {
		RunSaveAndLoadSummaryStateTest(PrepareNonexistentDirectory);
	}

	TEST(TEST_CLASS, CanSaveAndLoadSummaryState_DirectoryExists) {
		RunSaveAndLoadSummaryStateTest(PrepareEmptyDirectory);
	}

	// endregion

	// region LocalNodeStateSerializer::moveTo

	namespace {
		template<typename TPrepare>
		void RunMoveToTest(TPrepare prepare) {
			// Arrange: write a file in the source directory
			test::TempDirectoryGuard tempDir;
			auto stateDirectory = config::CatapultDirectory(tempDir.name() + "/zstate");
			boost::filesystem::create_directories(stateDirectory.path());
			io::IndexFile(stateDirectory.file("sentinel")).set(123);

			// - run additional preparation on destination directory
			auto stateDirectory2 = config::CatapultDirectory(tempDir.name() + "/zstate2");
			prepare(stateDirectory2);

			// Act:
			LocalNodeStateSerializer serializer(stateDirectory);
			serializer.moveTo(stateDirectory2);

			// Assert:
			EXPECT_FALSE(boost::filesystem::exists(stateDirectory.file("sentinel")));
			EXPECT_TRUE(boost::filesystem::exists(stateDirectory2.file("sentinel")));

			EXPECT_EQ(123u, io::IndexFile(stateDirectory2.file("sentinel")).get());
		}
	}

	TEST(TEST_CLASS, CanMoveTo_DirectoryDoesNotExist) {
		RunMoveToTest(PrepareNonexistentDirectory);
	}

	TEST(TEST_CLASS, CanMoveTo_EmptyDirectoryExists) {
		RunMoveToTest(PrepareEmptyDirectory);
	}

	TEST(TEST_CLASS, CanMoveTo_DirectoryWithFilesExists) {
		RunMoveToTest(PrepareDirectoryWithSentinel);
	}

	// endregion

	// region SaveStateToDirectoryWithCheckpointing

	namespace {
		auto ReadCommitStep(const config::CatapultDataDirectory& dataDirectory) {
			return static_cast<consumers::CommitOperationStep>(io::IndexFile(dataDirectory.rootDir().file("commit_step.dat")).get());
		}
	}

	TEST(TEST_CLASS, SaveStateToDirectoryWithCheckpointing_CommitStepIsBlocksWrittenWhenSaveFails) {
		// Arrange: indicate rocks is enabled
		test::TempDirectoryGuard tempDir;
		auto dataDirectory = config::CatapultDataDirectory(tempDir.name());
		auto nodeConfig = config::NodeConfiguration::Uninitialized();
		nodeConfig.EnableCacheDatabaseStorage = true;

		// - seed the cache state with rocks disabled
		auto blockChainConfig = model::BlockChainConfiguration::Uninitialized();
		auto catapultCache = test::CoreSystemCacheFactory::Create(blockChainConfig);
		auto supplementalData = CreateDeterministicSupplementalData();
		RandomSeedCache(catapultCache, supplementalData.State);

		// Act: save the state
		constexpr auto SaveState = SaveStateToDirectoryWithCheckpointing;
		EXPECT_THROW(SaveState(dataDirectory, nodeConfig, catapultCache, supplementalData.ChainScore), catapult_invalid_argument);

		// Assert:
		EXPECT_EQ(consumers::CommitOperationStep::Blocks_Written, ReadCommitStep(dataDirectory));
	}

	TEST(TEST_CLASS, SaveStateToDirectoryWithCheckpointing_CommitStepIsStateWrittenWhenMoveToFails) {
		// Arrange:
		test::TempDirectoryGuard tempDir;
		auto dataDirectory = config::CatapultDataDirectory(tempDir.name());
		auto nodeConfig = config::NodeConfiguration::Uninitialized();
		nodeConfig.EnableCacheDatabaseStorage = false;

		// - seed the cache state with rocks disabled
		auto blockChainConfig = model::BlockChainConfiguration::Uninitialized();
		auto catapultCache = test::CoreSystemCacheFactory::Create(blockChainConfig);
		auto supplementalData = CreateDeterministicSupplementalData();
		RandomSeedCache(catapultCache, supplementalData.State);

		// - trigger a moveTo failure
		io::IndexFile(dataDirectory.rootDir().file("state")).set(0);

		// Act: save the state
		constexpr auto SaveState = SaveStateToDirectoryWithCheckpointing;
		EXPECT_THROW(
				SaveState(dataDirectory, nodeConfig, catapultCache, supplementalData.ChainScore),
				boost::filesystem::filesystem_error);

		// Assert:
		EXPECT_EQ(consumers::CommitOperationStep::State_Written, ReadCommitStep(dataDirectory));
	}

	TEST(TEST_CLASS, SaveStateToDirectoryWithCheckpointing_CommitStepIsAllUpdatedWhenCatapultCacheSaveSucceeds) {
		// Arrange:
		test::TempDirectoryGuard tempDir;
		auto dataDirectory = config::CatapultDataDirectory(tempDir.name());
		auto nodeConfig = config::NodeConfiguration::Uninitialized();
		nodeConfig.EnableCacheDatabaseStorage = false;

		// - seed the cache state with rocks disabled
		auto blockChainConfig = model::BlockChainConfiguration::Uninitialized();
		auto catapultCache = test::CoreSystemCacheFactory::Create(blockChainConfig);
		auto supplementalData = CreateDeterministicSupplementalData();
		RandomSeedCache(catapultCache, supplementalData.State);

		// Act: save the state
		constexpr auto SaveState = SaveStateToDirectoryWithCheckpointing;
		SaveState(dataDirectory, nodeConfig, catapultCache, supplementalData.ChainScore);

		// Assert:
		EXPECT_EQ(consumers::CommitOperationStep::All_Updated, ReadCommitStep(dataDirectory));
	}

	TEST(TEST_CLASS, SaveStateToDirectoryWithCheckpointing_CommitStepIsAllUpdatedWhenCatapultCacheDeltaSaveSucceeds) {
		// Arrange:
		test::TempDirectoryGuard tempDir;
		auto dataDirectory = config::CatapultDataDirectory(tempDir.name());
		auto nodeConfig = config::NodeConfiguration::Uninitialized();
		nodeConfig.EnableCacheDatabaseStorage = true;

		// - seed the cache state with rocks enabled
		auto catapultCache = CreateCacheWithRealCoreSystemPlugins(tempDir.name() + "/db");
		auto supplementalData = CreateDeterministicSupplementalData();
		RandomSeedCache(catapultCache, supplementalData.State);

		// Act: save the state
		constexpr auto SaveState = SaveStateToDirectoryWithCheckpointing;
		SaveState(dataDirectory, nodeConfig, catapultCache, supplementalData.ChainScore);

		// Assert:
		EXPECT_EQ(consumers::CommitOperationStep::All_Updated, ReadCommitStep(dataDirectory));
	}

	// endregion
}}
