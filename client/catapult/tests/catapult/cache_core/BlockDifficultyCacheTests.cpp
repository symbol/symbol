#include "catapult/cache_core/BlockDifficultyCache.h"
#include "tests/test/cache/CacheBasicTests.h"
#include "tests/test/cache/CacheMixinsTests.h"
#include <vector>

namespace catapult { namespace cache {

#define TEST_CLASS BlockDifficultyCacheTests

	namespace {
		state::BlockDifficultyInfo CreateInfo(size_t id) {
			return state::BlockDifficultyInfo(Height(id), Timestamp(id), Difficulty(id));
		}
	}

	// region mixin traits based tests

	namespace {
		struct BlockDifficultyCacheMixinTraits {
			class CacheType : public BlockDifficultyCache {
			public:
				CacheType() : BlockDifficultyCache(300)
				{}
			};

			using IdType = state::BlockDifficultyInfo;
			using ValueType = state::BlockDifficultyInfo;

			static uint8_t GetRawId(const IdType& id) {
				return static_cast<uint8_t>(id.BlockHeight.unwrap());
			}

			static const IdType& GetId(const ValueType& value) {
				return value;
			}

			static IdType MakeId(uint8_t id) {
				return CreateInfo(id);
			}

			static ValueType CreateWithId(uint8_t id) {
				return MakeId(id);
			}
		};
	}

	DEFINE_CACHE_CONTAINS_TESTS_ORDERING(BlockDifficultyCacheMixinTraits, ViewAccessor, Strictly_Ordered, _View);
	DEFINE_CACHE_CONTAINS_TESTS_ORDERING(BlockDifficultyCacheMixinTraits, DeltaAccessor, Strictly_Ordered, _Delta);

	DEFINE_CACHE_ITERATION_TESTS_ORDERING(BlockDifficultyCacheMixinTraits, ViewAccessor, Strictly_Ordered, _View);

	DEFINE_CACHE_BASIC_TESTS(BlockDifficultyCacheMixinTraits,);

	// endregion

	// *** custom tests ***

	namespace {
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

		// Act + Assert:
		EXPECT_THROW(delta->insert(CreateInfo(1)), catapult_invalid_argument);
		EXPECT_THROW(delta->insert(CreateInfo(17)), catapult_invalid_argument);
	}

	TEST(TEST_CLASS, CanInsertElementIfOriginalSetIsEmpty) {
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

	TEST(TEST_CLASS, CanInsertElementIfOriginalSetIsNonEmpty) {
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

	TEST(TEST_CLASS, CanInsertElementAtArbitraryHeightIfOriginalSetIsEmpty) {
		// Arrange:
		BlockDifficultyCache cache(300);
		auto delta = cache.createDelta();

		// Act: add elements with heights starting at 117
		delta->insert(CreateInfo(117));
		delta->insert(CreateInfo(118));

		// Assert:
		EXPECT_EQ(2u, delta->size());
		EXPECT_TRUE(delta->contains(CreateInfo(117)));
		EXPECT_TRUE(delta->contains(CreateInfo(118)));
	}

	TEST(TEST_CLASS, CanInsertElementAtArbitraryHeightIfOriginalSetIsEmptied) {
		// Arrange: create a delta around a cache with 3 elements
		BlockDifficultyCache cache(300);
		SeedCache(cache, 3);
		auto delta = cache.createDelta();

		// Act: empty the cache
		delta->remove(CreateInfo(3));
		delta->remove(CreateInfo(2));
		delta->remove(CreateInfo(1));

		// - add elements with heights starting at 211
		delta->insert(CreateInfo(211));
		delta->insert(CreateInfo(212));

		// Assert:
		EXPECT_EQ(2u, delta->size());
		EXPECT_TRUE(delta->contains(CreateInfo(211)));
		EXPECT_TRUE(delta->contains(CreateInfo(212)));
	}

	// endregion

	// region remove

	TEST(TEST_CLASS, CanRemoveElementIfOriginalSetIsEmpty) {
		// Arrange:
		BlockDifficultyCache cache(300);
		auto delta = cache.createDelta();
		auto element1 = CreateInfo(1);
		auto element2 = CreateInfo(2);
		delta->insert(element1);
		delta->insert(element2);

		// Sanity check:
		EXPECT_EQ(2u, delta->size());
		EXPECT_TRUE(delta->contains(element1));
		EXPECT_TRUE(delta->contains(element2));

		// Act:
		delta->remove(element2);

		// Assert:
		EXPECT_EQ(1u, delta->size());
		EXPECT_TRUE(delta->contains(element1));
		EXPECT_FALSE(delta->contains(element2));
	}

	TEST(TEST_CLASS, CanRemoveElementIfOriginalSetIsNonEmpty) {
		// Arrange:
		BlockDifficultyCache cache(300);
		SeedCache(cache, 3);
		auto delta = cache.createDelta();
		auto element = CreateInfo(3);

		// Act:
		delta->remove(element);

		// Assert:
		EXPECT_EQ(2u, delta->size());
		EXPECT_FALSE(delta->contains(element));
	}

	TEST(TEST_CLASS, RemovingElementAndInsertingNewElementWithSameHeightUpdatesCacheWithNewElement) {
		// Arrange:
		BlockDifficultyCache cache(300);
		SeedCache(cache, 3);
		auto element = CreateInfo(3);
		auto newElement = CreateInfo(3);
		newElement.BlockTimestamp = Timestamp(123);

		// Sanity: initial timestamp is 3
		{
			auto view = cache.createView();
			auto infoRange = view->difficultyInfos(Height(3), 1);
			auto foundElement = *infoRange.begin();
			EXPECT_EQ(Height(3), foundElement.BlockHeight);
			EXPECT_EQ(Timestamp(3), foundElement.BlockTimestamp);
		}

		// Act:
		{
			auto delta = cache.createDelta();
			delta->remove(element);
			delta->insert(newElement);
			cache.commit();
		}

		auto view = cache.createView();
		auto infoRange = view->difficultyInfos(Height(3), 1);
		auto foundElement = *infoRange.begin();

		// Assert: new timestamp is 123
		EXPECT_EQ(3u, view->size());
		EXPECT_EQ(Height(3), foundElement.BlockHeight);
		EXPECT_EQ(Timestamp(123), foundElement.BlockTimestamp);
	}

	TEST(TEST_CLASS, RemoveThrowsIfElementHasUnexpectedHeight) {
		// Arrange:
		BlockDifficultyCache cache(300);
		SeedCache(cache, 10);
		auto delta = cache.createDelta();

		// Act + Assert:
		EXPECT_THROW(delta->remove(CreateInfo(5)), catapult_invalid_argument);
		EXPECT_THROW(delta->remove(CreateInfo(17)), catapult_invalid_argument);
	}

	TEST(TEST_CLASS, RemoveThrowsIfCacheIsEmpty) {
		// Arrange:
		BlockDifficultyCache cache(300);
		auto delta = cache.createDelta();

		// Act + Assert:
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

		// Act + Assert:
		EXPECT_THROW(view->difficultyInfos(Height(78), 35), catapult_runtime_error);
	}

	TEST(TEST_CLASS, DifficultyInfosThrowsIfHeightOrCountIsZero) {
		// Arrange:
		BlockDifficultyCache cache(300);
		SeedCache(cache, 100);
		auto view = cache.createView();

		// Act + Assert:
		EXPECT_THROW(view->difficultyInfos(Height(0), 1), catapult_invalid_argument);
		EXPECT_THROW(view->difficultyInfos(Height(50), 0), catapult_invalid_argument);
	}

	TEST(TEST_CLASS, DifficultyInfosThrowsIfInfoWithSpecifiedHeightIsNotFound) {
		// Arrange:
		BlockDifficultyCache cache(300);
		SeedCache(cache, 100);
		auto view = cache.createView();

		// Act + Assert:
		EXPECT_THROW(view->difficultyInfos(Height(101), 1), catapult_invalid_argument);
		EXPECT_THROW(view->difficultyInfos(Height(102), 1), catapult_invalid_argument);
		EXPECT_THROW(view->difficultyInfos(Height(1000), 1), catapult_invalid_argument);
	}

	// endregion
}}
