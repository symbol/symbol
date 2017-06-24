#include "src/cache/BlockDifficultyCache.h"
#include "tests/test/cache/CacheContentsTests.h"
#include "tests/test/cache/CacheIterationTests.h"
#include "tests/test/cache/CacheSynchronizationTests.h"
#include <vector>

#define TEST_CLASS BlockDifficultyCacheTests

namespace catapult { namespace cache {
	namespace {
		state::BlockDifficultyInfo CreateInfo(size_t id) {
			return state::BlockDifficultyInfo(Height(id), Timestamp(id), Difficulty(id));
		}

		void SeedCache(BlockDifficultyCache& cache, size_t count) {
			auto delta = cache.createDelta();
			for (auto i = 1u; i <= count; ++i)
				delta->insert(CreateInfo(i));

			cache.commit();
		}
	}

	// region insert

	TEST(TEST_CLASS, InsertThrowsIfElementHasUnexpectedHeight) {
		// Arrange:
		BlockDifficultyCache cache(300);
		SeedCache(cache, 10);
		auto delta = cache.createDelta();

		// Assert:
		EXPECT_THROW(delta->insert(CreateInfo(1)), catapult_invalid_argument);
		EXPECT_THROW(delta->insert(CreateInfo(17)), catapult_invalid_argument);
	}

	TEST(TEST_CLASS, CanInsertEntityIfOriginalSetIsEmpty) {
		// Arrange:
		BlockDifficultyCache cache(300);
		auto delta = cache.createDelta();

		// Act:
		delta->insert(CreateInfo(1));
		delta->insert(CreateInfo(2));

		// Assert:
		EXPECT_EQ(2u, delta->size());
		EXPECT_TRUE(delta->contains(CreateInfo(1)));
		EXPECT_TRUE(delta->contains(CreateInfo(2)));
	}

	TEST(TEST_CLASS, CanInsertEntityIfOriginalSetIsNonEmpty) {
		// Arrange:
		BlockDifficultyCache cache(300);
		SeedCache(cache, 3);
		auto delta = cache.createDelta();

		// Act:
		delta->insert(CreateInfo(4));
		delta->insert(CreateInfo(5));

		// Assert:
		EXPECT_EQ(5u, delta->size());
		EXPECT_TRUE(delta->contains(CreateInfo(4)));
		EXPECT_TRUE(delta->contains(CreateInfo(5)));
	}

	TEST(TEST_CLASS, CanInsertEntityAtArbitraryHeightIfOriginalSetIsEmpty) {
		// Arrange:
		BlockDifficultyCache cache(300);
		auto delta = cache.createDelta();

		// Act: add entities with heights starting at 117
		delta->insert(CreateInfo(117));
		delta->insert(CreateInfo(118));

		// Assert:
		EXPECT_EQ(2u, delta->size());
		EXPECT_TRUE(delta->contains(CreateInfo(117)));
		EXPECT_TRUE(delta->contains(CreateInfo(118)));
	}

	TEST(TEST_CLASS, CanInsertEntityAtArbitraryHeightIfOriginalSetIsEmptied) {
		// Arrange: create a delta around a cache with 3 entities
		BlockDifficultyCache cache(300);
		SeedCache(cache, 3);
		auto delta = cache.createDelta();

		// Act: empty the cache
		delta->remove(CreateInfo(3));
		delta->remove(CreateInfo(2));
		delta->remove(CreateInfo(1));

		// - add entities with heights starting at 211
		delta->insert(CreateInfo(211));
		delta->insert(CreateInfo(212));

		// Assert:
		EXPECT_EQ(2u, delta->size());
		EXPECT_TRUE(delta->contains(CreateInfo(211)));
		EXPECT_TRUE(delta->contains(CreateInfo(212)));
	}

	// endregion

	// region remove

	TEST(TEST_CLASS, CanRemoveEntityIfOriginalSetIsEmpty) {
		// Arrange:
		BlockDifficultyCache cache(300);
		auto delta = cache.createDelta();
		auto entity1 = CreateInfo(1);
		auto entity2 = CreateInfo(2);
		delta->insert(entity1);
		delta->insert(entity2);

		// Sanity check:
		EXPECT_EQ(2u, delta->size());
		EXPECT_TRUE(delta->contains(entity1));
		EXPECT_TRUE(delta->contains(entity2));

		// Act:
		delta->remove(entity2);

		// Assert:
		EXPECT_EQ(1u, delta->size());
		EXPECT_TRUE(delta->contains(entity1));
		EXPECT_FALSE(delta->contains(entity2));
	}

	TEST(TEST_CLASS, CanRemoveEntityIfOriginalSetIsNonEmpty) {
		// Arrange:
		BlockDifficultyCache cache(300);
		SeedCache(cache, 3);
		auto delta = cache.createDelta();
		auto entity = CreateInfo(3);

		// Act:
		delta->remove(entity);

		// Assert:
		EXPECT_EQ(2u, delta->size());
		EXPECT_FALSE(delta->contains(entity));
	}

	TEST(TEST_CLASS, RemovingEntityAndInsertingNewEntityWithSameHeightUpdatesCacheWithNewEntity) {
		// Arrange:
		BlockDifficultyCache cache(300);
		SeedCache(cache, 3);
		auto entity = CreateInfo(3);
		auto newEntity = CreateInfo(3);
		newEntity.BlockTimestamp = Timestamp(123);

		// Sanity: initial timestamp is 3
		{
			auto view = cache.createView();
			auto infoRange = view->difficultyInfos(Height(3), 1);
			auto foundEntity = *infoRange.begin();
			EXPECT_EQ(Height(3), foundEntity.BlockHeight);
			EXPECT_EQ(Timestamp(3), foundEntity.BlockTimestamp);
		}

		// Act:
		{
			auto delta = cache.createDelta();
			delta->remove(entity);
			delta->insert(newEntity);
			cache.commit();
		}

		auto view = cache.createView();
		auto infoRange = view->difficultyInfos(Height(3), 1);
		auto foundEntity = *infoRange.begin();

		// Assert: new timestamp is 123
		EXPECT_EQ(3u, view->size());
		EXPECT_EQ(Height(3), foundEntity.BlockHeight);
		EXPECT_EQ(Timestamp(123), foundEntity.BlockTimestamp);
	}

	TEST(TEST_CLASS, RemoveThrowsIfElementHasUnexpectedHeight) {
		// Arrange:
		BlockDifficultyCache cache(300);
		SeedCache(cache, 10);
		auto delta = cache.createDelta();

		// Assert:
		EXPECT_THROW(delta->remove(CreateInfo(5)), catapult_invalid_argument);
		EXPECT_THROW(delta->remove(CreateInfo(17)), catapult_invalid_argument);
	}

	TEST(TEST_CLASS, RemoveThrowsIfCacheIsEmpty) {
		// Arrange:
		BlockDifficultyCache cache(300);
		auto delta = cache.createDelta();

		// Assert:
		EXPECT_THROW(delta->remove(CreateInfo(5)), catapult_runtime_error);
		EXPECT_THROW(delta->remove(CreateInfo(17)), catapult_runtime_error);
	}

	// endregion

	// region prune

	TEST(TEST_CLASS, PruningBoundaryIsInitiallyUnset) {
		// Arrange:
		BlockDifficultyCache cache(300);
		auto delta = cache.createDelta();

		// Assert:
		EXPECT_FALSE(delta->pruningBoundary().isSet());
	}

	TEST(TEST_CLASS, PruneUpdatesPruningBoundary) {
		// Arrange:
		BlockDifficultyCache cache(420);
		auto delta = cache.createDelta();

		// Act:
		delta->prune(Height(123 + 420));
		auto pruningBoundary = delta->pruningBoundary();

		// Assert:
		EXPECT_TRUE(delta->pruningBoundary().isSet());
		auto info = pruningBoundary.value();
		EXPECT_EQ(Height(123), info.BlockHeight);
		EXPECT_EQ(Timestamp(0), info.BlockTimestamp);
		EXPECT_EQ(Difficulty(0), info.BlockDifficulty);
	}

	// endregion

	// region difficultyInfos

	TEST(TEST_CLASS, DifficultyInfosReturnsExpectedRange) {
		// Arrange:
		BlockDifficultyCache cache(300);
		SeedCache(cache, 100);
		size_t begin = 78 - 35 + 1;
		size_t end = 79;
		auto beginInfo = state::BlockDifficultyInfo(Height(begin));
		auto endInfo = state::BlockDifficultyInfo(Height(end));

		// Act:
		auto view = cache.createView();
		auto infoRange = view->difficultyInfos(Height(78), 35);

		// Assert:
		EXPECT_EQ(beginInfo, *infoRange.begin());
		EXPECT_EQ(endInfo, *infoRange.end());
	}

	TEST(TEST_CLASS, DifficultyInfosFirstIteratorPointsToSmallestInfoIfNotEnoughInfosAreAvailable) {
		// Arrange:
		BlockDifficultyCache cache(300);
		SeedCache(cache, 100);
		auto beginInfo = state::BlockDifficultyInfo(Height(1));
		auto endInfo = state::BlockDifficultyInfo(Height(31));

		// Act:
		auto view = cache.createView();
		auto infoRange = view->difficultyInfos(Height(30), 100);

		// Assert:
		EXPECT_EQ(beginInfo, *infoRange.begin());
		EXPECT_EQ(endInfo, *infoRange.end());
	}

	TEST(TEST_CLASS, DifficultyInfosThrowsIfCacheIsEmpty) {
		// Arrange:
		BlockDifficultyCache cache(300);
		auto view = cache.createView();

		// Assert:
		EXPECT_THROW(view->difficultyInfos(Height(78), 35), catapult_runtime_error);
	}

	TEST(TEST_CLASS, DifficultyInfosThrowsIfHeightOrCountIsZero) {
		// Arrange:
		BlockDifficultyCache cache(300);
		SeedCache(cache, 100);
		auto view = cache.createView();

		// Assert:
		EXPECT_THROW(view->difficultyInfos(Height(0), 1), catapult_invalid_argument);
		EXPECT_THROW(view->difficultyInfos(Height(50), 0), catapult_invalid_argument);
	}

	TEST(TEST_CLASS, DifficultyInfosThrowsIfInfoWithSpecifiedHeightIsNotFound) {
		// Arrange:
		BlockDifficultyCache cache(300);
		SeedCache(cache, 100);
		auto view = cache.createView();

		// Assert:
		EXPECT_THROW(view->difficultyInfos(Height(101), 1), catapult_invalid_argument);
		EXPECT_THROW(view->difficultyInfos(Height(102), 1), catapult_invalid_argument);
		EXPECT_THROW(view->difficultyInfos(Height(1000), 1), catapult_invalid_argument);
	}

	// endregion

	// region general cache tests

	namespace {
		struct BlockDifficultyCacheEntityTraits {
		public:
			static auto CreateEntity(size_t id) {
				return CreateInfo(id);
			}

			static const auto& ToKey(const state::BlockDifficultyInfo& value) {
				return value;
			}
		};

		using BaseTraits = test::SetCacheTraits<BlockDifficultyCache, state::BlockDifficultyInfo, BlockDifficultyCacheEntityTraits>;

		struct BlockDifficultyCacheTraits : BaseTraits {
		public:
			template<typename TAction>
			static void RunEmptyCacheTest(TAction action) {
				// Arrange:
				BlockDifficultyCache cache(300);

				// Act:
				action(cache);
			}

			template<typename TAction>
			static void RunCacheTest(TAction action) {
				// Arrange:
				BlockDifficultyCache cache(300);
				auto entities = InsertMultiple(cache, { 1, 2, 3 });

				// Act:
				action(cache, entities);
			}
		};
	}

	DEFINE_CACHE_CONTENTS_TESTS(TEST_CLASS, BlockDifficultyCacheTraits)
	DEFINE_CACHE_ITERATION_TESTS(TEST_CLASS, BlockDifficultyCacheTraits, Ordered)
	DEFINE_CACHE_SYNC_TESTS(TEST_CLASS, BlockDifficultyCacheTraits)

	// endregion
}}
