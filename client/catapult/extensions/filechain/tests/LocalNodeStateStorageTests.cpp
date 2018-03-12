#include "filechain/src/LocalNodeStateStorage.h"
#include "catapult/cache/CatapultCache.h"
#include "catapult/cache/SupplementalData.h"
#include "catapult/cache_core/AccountStateCache.h"
#include "catapult/cache_core/BlockDifficultyCache.h"
#include "catapult/io/FileLock.h"
#include "catapult/model/Address.h"
#include "catapult/model/BlockChainConfiguration.h"
#include "tests/test/cache/CacheTestUtils.h"
#include "tests/test/core/AccountStateTestUtils.h"
#include "tests/test/local/LocalTestUtils.h"
#include "tests/test/nodeps/Filesystem.h"
#include "tests/TestHarness.h"
#include <boost/filesystem.hpp>

namespace catapult { namespace filechain {

#define TEST_CLASS LocalNodeStateStorageTests

	namespace {
		constexpr model::NetworkIdentifier Default_Network_Id = model::NetworkIdentifier::Mijin_Test;
		constexpr size_t Account_Cache_Size = 123;
		constexpr size_t Block_Cache_Size = 200;

		void PopulateAccountStateCache(cache::AccountStateCacheDelta& cacheDelta) {
			for (auto i = 2u; i < Account_Cache_Size + 2; ++i) {
				auto publicKey = test::GenerateRandomData<Key_Size>();
				auto& accountState = 0 == i % 2
						? cacheDelta.addAccount(publicKey, Height(i / 2))
						: cacheDelta.addAccount(model::PublicKeyToAddress(publicKey, Default_Network_Id), Height(i / 2));

				test::RandomFillAccountData(i, accountState);
			}
		}

		void PopulateBlockDifficultyCache(cache::BlockDifficultyCacheDelta& cacheDelta) {
			for (auto i = 0u; i < Block_Cache_Size; ++i)
				cacheDelta.insert(Height(i), Timestamp(2 * i + 1), Difficulty(3 * i + 1));
		}

		void SanityAssertCache(const cache::CatapultCache& catapultCache) {
			auto view = catapultCache.createView();
			EXPECT_EQ(Account_Cache_Size, view.sub<cache::AccountStateCache>().size());
			EXPECT_EQ(Block_Cache_Size, view.sub<cache::BlockDifficultyCache>().size());
		}

		void AssertSubCaches(const cache::CatapultCache& expectedCache, const cache::CatapultCache& actualCache) {
			auto expectedView = expectedCache.createView();
			auto actualView = actualCache.createView();

			EXPECT_EQ(expectedView.sub<cache::AccountStateCache>().size(), actualView.sub<cache::AccountStateCache>().size());
			EXPECT_EQ(expectedView.sub<cache::BlockDifficultyCache>().size(), actualView.sub<cache::BlockDifficultyCache>().size());
		}

		cache::SupplementalData SaveState(const std::string& dataDirectory, cache::CatapultCache& cache) {
			cache::SupplementalData supplementalData;
			{
				auto delta = cache.createDelta();
				PopulateAccountStateCache(delta.sub<cache::AccountStateCache>());
				PopulateBlockDifficultyCache(delta.sub<cache::BlockDifficultyCache>());
				cache.commit(Height(54321));
			}

			// Sanity:
			SanityAssertCache(cache);

			supplementalData.ChainScore = model::ChainScore(0x1234567890ABCDEF, 0xFEDCBA0987654321);
			supplementalData.State.LastRecalculationHeight = model::ImportanceHeight(12345);
			filechain::SaveState(dataDirectory, cache, supplementalData);
			return supplementalData;
		}
	}

	TEST(TEST_CLASS, CanSaveAndLoadState) {
		// Arrange: seed and save the cache state
		test::TempDirectoryGuard tempDir;
		auto originalCache = test::CoreSystemCacheFactory::Create(model::BlockChainConfiguration::Uninitialized());
		auto originalSupplementalData = SaveState(tempDir.name(), originalCache);

		// Act: load the cache
		auto cache = test::CoreSystemCacheFactory::Create(model::BlockChainConfiguration::Uninitialized());
		cache::SupplementalData supplementalData;
		auto isStateLoaded = LoadState(tempDir.name(), cache, supplementalData);

		// Assert:
		EXPECT_TRUE(isStateLoaded);
		AssertSubCaches(originalCache, cache);
		EXPECT_EQ(originalSupplementalData.ChainScore, supplementalData.ChainScore);
		EXPECT_EQ(originalSupplementalData.State.LastRecalculationHeight, supplementalData.State.LastRecalculationHeight);
		EXPECT_EQ(Height(54321), cache.createView().height());
	}

	namespace {
		template<typename TAction>
		void AssertLoadStateFailure(const std::string& dataDirectory, TAction corruptSavedState) {
			// Arrange: seed and save the cache state
			auto originalCache = test::CoreSystemCacheFactory::Create(model::BlockChainConfiguration::Uninitialized());
			SaveState(dataDirectory, originalCache);

			// - corrupt the saved state
			corruptSavedState();

			// Act: load the cache
			auto cache = test::CoreSystemCacheFactory::Create(model::BlockChainConfiguration::Uninitialized());
			cache::SupplementalData supplementalData;
			auto isStateLoaded = LoadState(dataDirectory, cache, supplementalData);

			// Assert:
			EXPECT_FALSE(isStateLoaded);
		}
	}

	TEST(TEST_CLASS, CannotLoadStateIfSupplementalDataFileIsNotPresent) {
		// Arrange:
		test::TempDirectoryGuard tempDir;

		// Assert:
		AssertLoadStateFailure(tempDir.name(), [dataDirectory = tempDir.name()]() {
			// Arrange: remove the supplemental data file
			auto supplementalDataPath = boost::filesystem::path(dataDirectory) / "state" / "supplemental.dat";
			ASSERT_TRUE(boost::filesystem::remove(supplementalDataPath));
		});
	}

	TEST(TEST_CLASS, CannotLoadStateIfLockFileIsPresent) {
		// Arrange:
		test::TempDirectoryGuard tempDir;

		// Assert:
		AssertLoadStateFailure(tempDir.name(), [dataDirectory = tempDir.name()]() {
			// Arrange: simulate a lock file
			auto lockFilePath = boost::filesystem::path(dataDirectory) / "state" / "state.lock";
			std::ofstream lockFileStream(lockFilePath.generic_string());
		});
	}

	TEST(TEST_CLASS, SaveStateIgnoresLockFile) {
		// Arrange:
		test::TempDirectoryGuard tempDir;

		// - simulate a lock file
		auto lockFilePath = boost::filesystem::path(tempDir.name()) / "state" / "state.lock";
		ASSERT_TRUE(boost::filesystem::create_directory(boost::filesystem::path(tempDir.name()) / "state"));
		{
			// - create the file and immediately close it in order to allow this test to pass on windows
			std::ofstream lockFileStream(lockFilePath.generic_string());
		}

		// - seed and save the cache state in the presence of a lock file
		auto originalCache = test::CoreSystemCacheFactory::Create(model::BlockChainConfiguration::Uninitialized());
		auto originalSupplementalData = SaveState(tempDir.name(), originalCache);

		// Sanity: the lock file should have been removed by SaveState
		EXPECT_FALSE(boost::filesystem::exists(lockFilePath));

		// Act: load the cache
		auto cache = test::CoreSystemCacheFactory::Create(model::BlockChainConfiguration::Uninitialized());
		cache::SupplementalData supplementalData;
		auto isStateLoaded = LoadState(tempDir.name(), cache, supplementalData);

		// Assert:
		EXPECT_TRUE(isStateLoaded);
		AssertSubCaches(originalCache, cache);
		EXPECT_EQ(originalSupplementalData.ChainScore, supplementalData.ChainScore);
		EXPECT_EQ(originalSupplementalData.State.LastRecalculationHeight, supplementalData.State.LastRecalculationHeight);
		EXPECT_EQ(Height(54321), cache.createView().height());
	}
}}
