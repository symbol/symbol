#include "src/observers/Observers.h"
#include "src/cache/MosaicCache.h"
#include "tests/test/MosaicCacheTestUtils.h"
#include "tests/test/core/NotificationTestUtils.h"
#include "tests/test/plugins/ObserverTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace observers {
	using ObserverTestContext = test::ObserverTestContextT<test::MosaicCacheFactory>;

	DEFINE_COMMON_OBSERVER_TESTS(MosaicPruning, 123)

	namespace {
		constexpr uint32_t Max_Rollback_Blocks = 20;

		void SeedCache(cache::CatapultCacheDelta& cache) {
			test::AddMosaic(cache, MosaicId(26), Height(500), ArtifactDuration(100), Amount());
			test::AddMosaic(cache, MosaicId(37), Height(500), ArtifactDuration(150), Amount());
			test::AddMosaic(cache, MosaicId(38), Height(500), ArtifactDuration(150), Amount());
			test::AddMosaic(cache, MosaicId(39), Height(500), ArtifactDuration(150), Amount());
			test::AddMosaic(cache, MosaicId(48), Height(500), ArtifactDuration(200), Amount());
		}

		template<typename TSeedCacheFunc, typename TCheckCacheFunc>
		void RunTest(ObserverTestContext&& context, TSeedCacheFunc seedCache, TCheckCacheFunc checkCache) {
			// Arrange:
			auto pObserver = CreateMosaicPruningObserver(Max_Rollback_Blocks);
			auto notification = test::CreateBlockNotification();

			// - seed the cache
			seedCache(context.cache());

			// Act:
			test::ObserveNotification(*pObserver, notification, context);

			// Assert: check the cache
			checkCache(context.cache().sub<cache::MosaicCache>());
		}
	}

	// region no operation

	TEST(MosaicPruningObserverTests, ObserverDoesNothingInModeRollback) {
		// Act + Assert: id 26: to be pruned at height 500 + 100 + 20 = 620 if it was mode commit
		RunTest(
				ObserverTestContext(NotifyMode::Rollback, Height(620)),
				SeedCache,
				[](auto& mosaicCacheDelta) {
					test::AssertCacheContents(mosaicCacheDelta, { 26, 37, 38, 39, 48 });
				});
	}

	TEST(MosaicPruningObserverTests, ObserverDoesNothingIfPassedInHeightIsSmallerThanOrEqualToMaxRollbackBlocks) {
		// Act + Assert: maxRollbackBlocks is 20
		for (auto height : { 1u, 2u, 10u, 19u, 20u }) {
			RunTest(
					ObserverTestContext(NotifyMode::Commit, Height(height)),
					[](cache::CatapultCacheDelta& cacheDelta) {
						SeedCache(cacheDelta);
						test::AddMosaic(cacheDelta, MosaicId(13), Height(2), ArtifactDuration(10), Amount());

						// Sanity:
						const auto& mosaicCache = cacheDelta.sub<cache::MosaicCache>();
						EXPECT_EQ(6u, mosaicCache.deepSize());
					},
					[](auto& mosaicCacheDelta) {
						test::AssertCacheContents(mosaicCacheDelta, { 13, 26, 37, 38, 39, 48 });
					});
		}
	}

	// endregion

	// region pruning

	TEST(MosaicPruningObserverTests, ObserverCanPruneSingleExpiredMosaic_HistoryDepthOne) {
		// Act + Assert: id 26: to be pruned at height 500 + 100 + 20 = 620
		RunTest(
				ObserverTestContext(NotifyMode::Commit, Height(620)),
				SeedCache,
				[](auto& mosaicCacheDelta) {
					test::AssertCacheContents(mosaicCacheDelta, { 37, 38, 39, 48 });
				});
	}

	TEST(MosaicPruningObserverTests, ObserverCanPruneMultipleExpiredMosaics_HistoryDepthOne) {
		// Act + Assert:
		// - ids 37, 38, 39: to be pruned at height 500 + 150 + 20 = 670
		// - id 26: not pruned because it expires at 620
		RunTest(
				ObserverTestContext(NotifyMode::Commit, Height(670)),
				SeedCache,
				[](auto& mosaicCacheDelta) {
					test::AssertCacheContents(mosaicCacheDelta, { 26, 48 });
				});
	}

	TEST(MosaicPruningObserverTests, ObserverCanPruneSingleExpiredMosaic_HistoryDepthGreaterThanOne_LatestShortensDuration) {
		// Act + Assert: new version with id 26: to be pruned at height 520 + 70 + 20 = 610
		RunTest(
				ObserverTestContext(NotifyMode::Commit, Height(610)),
				[](cache::CatapultCacheDelta& cacheDelta) {
					SeedCache(cacheDelta);
					test::AddMosaic(cacheDelta, MosaicId(26), Height(520), ArtifactDuration(70), Amount());

					// Sanity:
					const auto& mosaicCache = cacheDelta.sub<cache::MosaicCache>();
					EXPECT_EQ(6u, mosaicCache.deepSize());
				},
				[](auto& mosaicCacheDelta) {
					test::AssertCacheContents(mosaicCacheDelta, { 37, 38, 39, 48 });

					// - both versions of the mosaic with id 26 should be gone
					EXPECT_EQ(4u, mosaicCacheDelta.deepSize());
				});
	}

	TEST(MosaicPruningObserverTests, ObserverCanPruneSingleExpiredMosaic_HistoryDepthGreaterThanOne_LatestExtendsDuration) {
		// Act + Assert: old version with id 26: to be pruned at height 500 + 100 + 20 = 620
		RunTest(
				ObserverTestContext(NotifyMode::Commit, Height(620)),
				[](cache::CatapultCacheDelta& cacheDelta) {
					SeedCache(cacheDelta);
					test::AddMosaic(cacheDelta, MosaicId(26), Height(520), ArtifactDuration(90), Amount());

					// Sanity:
					const auto& mosaicCache = cacheDelta.sub<cache::MosaicCache>();
					EXPECT_EQ(6u, mosaicCache.deepSize());
				},
				[](auto& mosaicCacheDelta) {
					test::AssertCacheContents(mosaicCacheDelta, { 26, 37, 38, 39, 48 });

					// - old version of the mosaic with id 26 should be gone
					EXPECT_EQ(5u, mosaicCacheDelta.deepSize());
				});
	}

	// endregion
}}
