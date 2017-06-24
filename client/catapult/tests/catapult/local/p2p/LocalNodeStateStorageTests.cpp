#include "catapult/local/p2p/LocalNodeStateStorage.h"
#include "catapult/cache/AccountStateCache.h"
#include "catapult/cache/BlockDifficultyCache.h"
#include "catapult/cache/CatapultCache.h"
#include "catapult/cache/SupplementalData.h"
#include "catapult/model/Address.h"
#include "catapult/model/BlockChainConfiguration.h"
#include "tests/test/cache/CacheTestUtils.h"
#include "tests/test/core/AccountStateTestUtils.h"
#include "tests/test/local/LocalTestUtils.h"
#include "tests/test/nodeps/Filesystem.h"
#include "tests/TestHarness.h"

namespace catapult { namespace local { namespace p2p {

	namespace {
		constexpr model::NetworkIdentifier Default_Network_Id = model::NetworkIdentifier::Mijin_Test;
		constexpr size_t Account_Cache_Size = 123;
		constexpr size_t Block_Cache_Size = 200;

		void PopulateAccountStateCache(cache::AccountStateCacheDelta& cacheDelta) {
			for (auto i = 2u; i < Account_Cache_Size + 2; ++i) {
				auto publicKey = test::GenerateRandomData<Key_Size>();
				auto pAccount = 0 == i % 2 ?
					cacheDelta.addAccount(publicKey, Height(i / 2)) :
					cacheDelta.addAccount(model::PublicKeyToAddress(publicKey, Default_Network_Id), Height(i / 2));

				test::RandomFillAccountData(i, *pAccount);
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
	}

	TEST(LocalNodeStateStorageTests, CanSaveAndLoadState) {
		// Arrange:
		test::TempDirectoryGuard tempDir;

		auto originalCache = test::CoreSystemCacheFactory::Create(model::BlockChainConfiguration::Uninitialized());
		cache::SupplementalData originalSupplementalData;
		{
			{
				auto delta = originalCache.createDelta();
				PopulateAccountStateCache(delta.sub<cache::AccountStateCache>());
				PopulateBlockDifficultyCache(delta.sub<cache::BlockDifficultyCache>());
				originalCache.commit(Height(54321));
			}

			// Sanity:
			SanityAssertCache(originalCache);

			originalSupplementalData.ChainScore = model::ChainScore(0x1234567890abcdef, 0xfedcba0987654321);
			originalSupplementalData.State.LastRecalculationHeight = model::ImportanceHeight(12345);
			SaveState(tempDir.name(), originalCache, originalSupplementalData);
		}

		// Act:
		auto cache = test::CoreSystemCacheFactory::Create(model::BlockChainConfiguration::Uninitialized());
		cache::SupplementalData supplementalData;
		LoadState(tempDir.name(), cache, supplementalData);

		// Assert:
		AssertSubCaches(originalCache, cache);
		EXPECT_EQ(originalSupplementalData.ChainScore, supplementalData.ChainScore);
		EXPECT_EQ(originalSupplementalData.State.LastRecalculationHeight, supplementalData.State.LastRecalculationHeight);
		EXPECT_EQ(Height(54321), cache.createView().height());
	}
}}}
