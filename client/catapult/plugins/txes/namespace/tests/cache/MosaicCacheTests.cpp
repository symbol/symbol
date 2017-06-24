#include "src/cache/MosaicCache.h"
#include "tests/test/MosaicCacheTestUtils.h"
#include "tests/test/MosaicTestUtils.h"
#include "tests/test/cache/CacheContentsTests.h"
#include "tests/test/cache/CacheIterationTests.h"
#include "tests/test/cache/CacheSynchronizationTests.h"
#include "tests/TestHarness.h"

#define TEST_CLASS MosaicCacheTests

namespace catapult { namespace cache {

	namespace {
		model::MosaicProperties CreatePropertiesWithDuration(uint64_t duration) {
			return test::CreateMosaicPropertiesWithDuration(ArtifactDuration(duration));
		}

		state::MosaicEntry CreateMosaicEntry(MosaicId id, Height height, uint64_t duration, Amount supply) {
			auto entry = state::MosaicEntry(
					NamespaceId(123),
					id,
					state::MosaicDefinition(height, test::GenerateRandomData<Key_Size>(), CreatePropertiesWithDuration(duration)));
			entry.increaseSupply(supply);
			return entry;
		}

		state::MosaicEntry CreateMosaicEntry(NamespaceId namespaceId, MosaicId id, Amount supply) {
			auto entry = state::MosaicEntry(
					namespaceId,
					id,
					state::MosaicDefinition(Height(234), test::GenerateRandomData<Key_Size>(), CreatePropertiesWithDuration(321 - 234)));
			entry.increaseSupply(supply);
			return entry;
		}

		void PopulateCache(LockedCacheDelta<MosaicCacheDelta>& delta) {
			// add five mosaic ids (namespace id 111)
			for (auto id : { 1u, 3u, 5u, 7u, 9u })
				delta->insert(CreateMosaicEntry(NamespaceId(111), MosaicId(id), Amount(id * id)));

			// add four more mosaic ids  (namespace id 222)
			for (auto id : { 2u, 4u, 6u, 8u })
				delta->insert(CreateMosaicEntry(NamespaceId(222), MosaicId(id), Amount(id * id)));

			// add an extra entry to id 3 (namespace id 111)
			delta->insert(CreateMosaicEntry(NamespaceId(111), MosaicId(3), Amount(27)));
		}
	}

	// region basic tests

	TEST(TEST_CLASS, CacheInitiallyContainsNoMosaics) {
		// Arrange:
		MosaicCache cache;
		auto view = cache.createView();

		// Assert:
		test::AssertCacheSizes(*view, 0, 0);
	}

	// note: basic contains tests are supplied by CacheContentsTests

	// endregion

	// region deep size

	TEST(TEST_CLASS, DeepSizeRespectsMosaicHistory_SingleItem) {
		// Arrange:
		MosaicCache cache;
		{
			// - insert three definitions for one mosaic
			auto delta = cache.createDelta();
			delta->insert(test::CreateMosaicEntry(MosaicId(234), Amount(1)));
			delta->insert(test::CreateMosaicEntry(MosaicId(234), Amount(2)));
			delta->insert(test::CreateMosaicEntry(MosaicId(234), Amount(3)));

			// Assert: one mosaic, three definitions
			test::AssertCacheSizes(*delta, 1, 3);

			cache.commit();
		}

		// Assert: one mosaic, three definitions
		auto view = cache.createView();
		test::AssertCacheSizes(*view, 1, 3);
	}

	TEST(TEST_CLASS, DeepSizeRespectsMosaicHistory_MultipleItems) {
		// Arrange:
		MosaicCache cache;
		{
			// - insert three definitions for one mosaic and two definitions for another
			auto delta = cache.createDelta();
			delta->insert(test::CreateMosaicEntry(MosaicId(234), Amount(1)));
			delta->insert(test::CreateMosaicEntry(MosaicId(432), Amount(1)));
			delta->insert(test::CreateMosaicEntry(MosaicId(234), Amount(2)));
			delta->insert(test::CreateMosaicEntry(MosaicId(432), Amount(2)));
			delta->insert(test::CreateMosaicEntry(MosaicId(234), Amount(3)));

			// Assert: two mosaics, five definitions
			test::AssertCacheSizes(*delta, 2, 5);

			cache.commit();
		}

		// Assert: two mosaics, five definitions
		auto view = cache.createView();
		test::AssertCacheSizes(*view, 2, 5);
	}

	// endregion

	// region DELTA_VIEW_BASED_TEST

	namespace {
		struct ViewTraits {
			template<typename TAction>
			static void RunTest(TAction action) {
				// Arrange:
				MosaicCache cache;
				{
					auto delta = cache.createDelta();
					PopulateCache(delta);
					cache.commit();
				}

				// Act:
				auto view = cache.createView();
				action(view);
			}
		};

		struct DeltaTraits {
			template<typename TAction>
			static void RunTest(TAction action) {
				// Arrange:
				MosaicCache cache;
				auto delta = cache.createDelta();
				PopulateCache(delta);

				// Act:
				action(delta);
			}
		};
	}

#define DELTA_VIEW_BASED_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_View) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<ViewTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Delta) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<DeltaTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	// endregion

	// region get

	DELTA_VIEW_BASED_TEST(GetReturnsKnownMosaicEntry) {
		// Act:
		TTraits::RunTest([](const auto& view) {
			const auto& entry = view->get(MosaicId(5));

			// Assert:
			EXPECT_EQ(MosaicId(5), entry.mosaicId());
			EXPECT_EQ(Amount(25), entry.supply());
		});
	}

	DELTA_VIEW_BASED_TEST(GetThrowsIfMosaicIsUnknown) {
		// Act:
		TTraits::RunTest([](const auto& view) {
			// Assert:
			EXPECT_THROW(view->get(MosaicId(123)), catapult_invalid_argument);
		});
	}

	TEST(TEST_CLASS, GetReturnsKnownMosaicEntry_DeltaMutable) {
		// Act:
		DeltaTraits::RunTest([](auto& view) {
			auto& entry = view->get(MosaicId(5));
			entry.increaseSupply(Amount(1));

			// Assert:
			EXPECT_EQ(MosaicId(5), entry.mosaicId());
			EXPECT_EQ(Amount(26), entry.supply());
		});
	}

	TEST(TEST_CLASS, GetThrowsIfMosaicIsUnknown_DeltaMutable) {
		// Act:
		DeltaTraits::RunTest([](auto& view) {
			// Assert:
			EXPECT_THROW(view->get(MosaicId(123)), catapult_invalid_argument);
		});
	}

	TEST(TEST_CLASS, GetConstAndNonConstOverloadsReturnReferencesToSameEntry) {
		// Act:
		DeltaTraits::RunTest([](auto& view) {
			auto& mutableEntry = view->get(MosaicId(5));
			auto& constEntry = const_cast<const MosaicCacheDelta&>(*view).get(MosaicId(5));

			// Assert:
			EXPECT_EQ(&mutableEntry, &constEntry);
		});
	}

	// endregion

	// region isActive

	DELTA_VIEW_BASED_TEST(IsActiveReturnsTrueForKnownActiveMosaic) {
		// Act:
		TTraits::RunTest([](const auto& view) {
			// Assert: mosaic with id 5 has lifetime (234, 321)
			EXPECT_TRUE(view->isActive(MosaicId(5), Height(234)));
			EXPECT_TRUE(view->isActive(MosaicId(5), Height(298)));
			EXPECT_TRUE(view->isActive(MosaicId(5), Height(320)));
		});
	}

	DELTA_VIEW_BASED_TEST(IsActiveReturnsFalseForUnknownMosaics) {
		// Act:
		TTraits::RunTest([](const auto& view) {
			// Assert:
			EXPECT_FALSE(view->isActive(MosaicId(123), Height(1)));
			EXPECT_FALSE(view->isActive(MosaicId(234), Height(2)));
			EXPECT_FALSE(view->isActive(MosaicId(345), Height(123)));
			EXPECT_FALSE(view->isActive(MosaicId(456), Height(10000)));
		});
	}

	DELTA_VIEW_BASED_TEST(IsActiveReturnsFalseForKnownInactiveMosaic) {
		// Act:
		TTraits::RunTest([](const auto& view) {
			// Assert: mosaic with id 5 has lifetime (234, 321)
			EXPECT_FALSE(view->isActive(MosaicId(5), Height(1)));
			EXPECT_FALSE(view->isActive(MosaicId(5), Height(233)));
			EXPECT_FALSE(view->isActive(MosaicId(5), Height(321)));
		});
	}

	// endregion

	// region insert

	TEST(TEST_CLASS, CanInsertEntryWithoutExistingHistory) {
		// Arrange:
		MosaicCache cache;
		auto delta = cache.createDelta();

		// Act:
		delta->insert(test::CreateMosaicEntry(MosaicId(234), Amount(1)));

		// Assert:
		test::AssertCacheSizes(*delta, 1, 1);
		EXPECT_TRUE(delta->contains(MosaicId(234)));
	}

	TEST(TEST_CLASS, CanInsertEntryWithExistingHistoryWithConsistentNamespace) {
		// Arrange:
		MosaicCache cache;
		{
			auto delta = cache.createDelta();
			delta->insert(CreateMosaicEntry(NamespaceId(111), MosaicId(234), Amount(7)));

			// Act: add additional history
			delta->insert(CreateMosaicEntry(NamespaceId(111), MosaicId(234), Amount(11)));
			delta->insert(CreateMosaicEntry(NamespaceId(111), MosaicId(234), Amount(13)));

			cache.commit();
		}

		// Assert:
		auto view = cache.createView();
		test::AssertCacheSizes(*view, 1, 3);
		ASSERT_TRUE(view->contains(MosaicId(234)));

		const auto& entry = view->get(MosaicId(234));
		EXPECT_EQ(NamespaceId(111), entry.namespaceId());
		EXPECT_EQ(MosaicId(234), entry.mosaicId());
		EXPECT_EQ(Amount(13), entry.supply());
	}

	TEST(TEST_CLASS, CannotInsertEntryWithExistingHistoryWithInconsistentNamespace) {
		// Arrange:
		MosaicCache cache;
		auto delta = cache.createDelta();
		delta->insert(CreateMosaicEntry(NamespaceId(111), MosaicId(234), Amount(7)));

		// Act: add additional history with an inconsistent namespace
		EXPECT_THROW(delta->insert(CreateMosaicEntry(NamespaceId(112), MosaicId(234), Amount(11))), catapult_runtime_error);
	}

	TEST(TEST_CLASS, CanInsertMultipleHistories) {
		// Arrange:
		MosaicCache cache;
		{
			auto delta = cache.createDelta();
			delta->insert(test::CreateMosaicEntry(MosaicId(234), Amount(7)));
			delta->insert(test::CreateMosaicEntry(MosaicId(123), Amount(16)));

			// Act: add additional history
			delta->insert(test::CreateMosaicEntry(MosaicId(234), Amount(11)));
			delta->insert(test::CreateMosaicEntry(MosaicId(123), Amount(9)));
			delta->insert(test::CreateMosaicEntry(MosaicId(234), Amount(13)));

			cache.commit();
		}

		// Assert:
		auto view = cache.createView();
		test::AssertCacheSizes(*view, 2, 5);
		ASSERT_TRUE(view->contains(MosaicId(123)));
		EXPECT_EQ(Amount(9), view->get(MosaicId(123)).supply());

		ASSERT_TRUE(view->contains(MosaicId(234)));
		EXPECT_EQ(Amount(13), view->get(MosaicId(234)).supply());
	}

	// endregion

	// region remove (MosaicId)

	TEST(TEST_CLASS, CannotRemoveUnknownMosaic) {
		// Arrange:
		MosaicCache cache;
		auto delta = cache.createDelta();
		PopulateCache(delta);

		// Assert:
		EXPECT_THROW(delta->remove(MosaicId(10)), catapult_invalid_argument);
		EXPECT_THROW(delta->remove(MosaicId(123)), catapult_invalid_argument);
		EXPECT_THROW(delta->remove(MosaicId(3579)), catapult_invalid_argument);
	}

	TEST(TEST_CLASS, CanRemoveKnownMosaicWithHistoryDepthOne) {
		// Arrange:
		MosaicCache cache;
		auto delta = cache.createDelta();
		PopulateCache(delta);

		// Sanity:
		test::AssertCacheSizes(*delta, 9, 10);
		EXPECT_TRUE(delta->contains(MosaicId(5)));

		// Act:
		delta->remove(MosaicId(5));

		// Assert:
		test::AssertCacheSizes(*delta, 8, 9);
		EXPECT_FALSE(delta->contains(MosaicId(5)));
	}

	TEST(TEST_CLASS, CanRemoveMostRecentEntryFromKnownMosaicWithHistoryDepthGreaterThanOne) {
		// Arrange:
		MosaicCache cache;
		auto delta = cache.createDelta();
		PopulateCache(delta);

		// Sanity:
		test::AssertCacheSizes(*delta, 9, 10);
		ASSERT_TRUE(delta->contains(MosaicId(3)));
		EXPECT_EQ(Amount(27), delta->get(MosaicId(3)).supply());

		// Act:
		delta->remove(MosaicId(3));

		// Assert:
		test::AssertCacheSizes(*delta, 9, 9);
		ASSERT_TRUE(delta->contains(MosaicId(3)));
		EXPECT_EQ(Amount(9), delta->get(MosaicId(3)).supply());
	}

	// endregion

	// region remove (NamespaceId)

	TEST(TEST_CLASS, CanRemoveNamespaceWithNoMosaicsWithNoEffect) {
		// Arrange:
		MosaicCache cache;
		auto delta = cache.createDelta();
		PopulateCache(delta);

		// Sanity:
		test::AssertCacheSizes(*delta, 9, 10);

		// Act:
		delta->remove(NamespaceId(10));
		delta->remove(NamespaceId(123));
		delta->remove(NamespaceId(3579));

		// Assert:
		test::AssertCacheSizes(*delta, 9, 10);
	}

	TEST(TEST_CLASS, CanRemoveKnownNamespaceContainingNoMosaicsWithHistoryDepthGreaterThanOne) {
		// Arrange:
		MosaicCache cache;
		auto delta = cache.createDelta();
		PopulateCache(delta);

		// Sanity:
		test::AssertCacheSizes(*delta, 9, 10);
		test::AssertCacheContents(*delta, { 1, 2, 3, 4, 5, 6, 7, 8, 9 });

		// Act:
		delta->remove(NamespaceId(222));

		// Assert:
		test::AssertCacheSizes(*delta, 5, 6);
		test::AssertCacheContents(*delta, { 1, 3, 5, 7, 9 });
	}

	TEST(TEST_CLASS, CanRemoveKnownNamespaceContainingMosaicsWithHistoryDepthGreaterThanOne) {
		// Arrange:
		MosaicCache cache;
		auto delta = cache.createDelta();
		PopulateCache(delta);

		// Sanity:
		test::AssertCacheSizes(*delta, 9, 10);
		test::AssertCacheContents(*delta, { 1, 2, 3, 4, 5, 6, 7, 8, 9 });

		// Act: note that MosaicId(3) has two history entries
		delta->remove(NamespaceId(111));

		// Assert:
		test::AssertCacheSizes(*delta, 4, 4);
		test::AssertCacheContents(*delta, { 2, 4, 6, 8 });
	}

	// endregion

	// region prune

	namespace {
		void SetupCacheForPruneTests(MosaicCache& cache) {
			// 10 mosaics with id i and lifetime (10 * i, 10 * (i + 1)) for i = 0, ..., 9
			constexpr size_t Mosaic_Count = 10;

			auto delta = cache.createDelta();
			for (auto i = 0u; i < Mosaic_Count; ++i)
				delta->insert(CreateMosaicEntry(MosaicId(i), Height(10 * i), 10, Amount(i * i)));

			// Sanity:
			test::AssertCacheSizes(*delta, 10, 10);

			cache.commit();
		}

		void PruneAll(cache::MosaicCacheDelta& delta, Height maxHeight) {
			// Act: increment by 10
			for (auto height = Height(10u); height <= maxHeight; height = height + Height(10))
				delta.prune(height);
		}
	}

	TEST(TEST_CLASS, CanPruneEmptyMosaicCache) {
		// Arrange:
		MosaicCache cache;
		auto delta = cache.createDelta();

		// Sanity:
		test::AssertCacheSizes(*delta, 0, 0);

		// Act: prune on empty cache does not throw
		delta->prune(Height(123));

		// Assert:
		test::AssertCacheSizes(*delta, 0, 0);
	}

	TEST(TEST_CLASS, PruneIsNoOpIfAllMosaicsExpireAfterPruneHeight) {
		// Arrange:
		MosaicCache cache;
		SetupCacheForPruneTests(cache);
		auto delta = cache.createDelta();

		// Act: first mosaic expires at height 10
		for (auto i = 0u; 10u > i; ++i)
			delta->prune(Height(i));

		// Assert: size has not changed
		test::AssertCacheSizes(*delta, 10, 10);
	}

	TEST(TEST_CLASS, PruneIsNoOpIfNoMosaicHasExpiredAtPruneHeights) {
		// Arrange:
		MosaicCache cache;
		SetupCacheForPruneTests(cache);
		auto delta = cache.createDelta();

		// Act: for heights 11 to 19 no mosaic expired
		for (auto i = 11u; 20u > i; ++i)
			delta->prune(Height(i));

		// Assert: size has not changed
		test::AssertCacheSizes(*delta, 10, 10);
	}

	TEST(TEST_CLASS, PruneRemovesExpiredMosaicsWhenHistoryDepthIsOne) {
		// Arrange:
		MosaicCache cache;
		SetupCacheForPruneTests(cache);
		auto delta = cache.createDelta();

		// Act: prune mosaics with id 0 - 6 which expire at heights 10, 20, ..., 70
		PruneAll(*delta, Height(70));
		cache.commit();

		// Assert:
		test::AssertCacheSizes(*delta, 3, 3);
		test::AssertCacheContents(*cache.createView(), { 7, 8, 9 });
	}

	TEST(TEST_CLASS, PruneRemovesExpiredMosaicsWhenHistoryDepthIsNotOneAndLatestExtendsDuration) {
		// Arrange:
		MosaicCache cache;
		SetupCacheForPruneTests(cache);
		auto delta = cache.createDelta();

		// - extend the duration of mosaic id 4
		delta->insert(CreateMosaicEntry(MosaicId(4), Height(100), 10, Amount(333)));

		// Sanity:
		test::AssertCacheSizes(*delta, 10, 11);

		// Act: prune mosaic with id 0 - 6 (the latest 4 entry was not pruned)
		PruneAll(*delta, Height(70));
		cache.commit();

		// Assert:
		test::AssertCacheSizes(*delta, 4, 4);
		test::AssertCacheContents(*cache.createView(), { 4, 7, 8, 9 });
	}

	TEST(TEST_CLASS, PruneRemovesExpiredMosaicsWhenHistoryDepthIsNotOneAndLatestShortensDuration) {
		// Arrange:
		MosaicCache cache;
		SetupCacheForPruneTests(cache);
		auto delta = cache.createDelta();

		// - shorten the duration of mosaic id 8
		delta->insert(CreateMosaicEntry(MosaicId(8), Height(50), 10, Amount(333)));

		// Sanity:
		test::AssertCacheSizes(*delta, 10, 11);

		// Act: prune mosaic with id 0 - 6 and 8 (the oldest 8 entry is pruned even though it is active)
		PruneAll(*delta, Height(70));
		cache.commit();

		// Assert:
		test::AssertCacheSizes(*delta, 2, 2);
		test::AssertCacheContents(*cache.createView(), { 7, 9 });
	}

	TEST(TEST_CLASS, CanPruneAllMosaics) {
		// Arrange:
		MosaicCache cache;
		SetupCacheForPruneTests(cache);
		auto delta = cache.createDelta();

		// Act: prune all mosaics
		PruneAll(*delta, Height(100));

		// Assert:
		test::AssertCacheSizes(*delta, 0, 0);
	}

	TEST(TEST_CLASS, PruneIsIdempotent) {
		// Arrange:
		MosaicCache cache;
		SetupCacheForPruneTests(cache);
		auto delta = cache.createDelta();

		// Act: prune mosaics with id 0 - 4
		for (auto i = 0u; 10 > i; ++i)
			PruneAll(*delta, Height(50));

		cache.commit();

		// Assert:
		test::AssertCacheSizes(*delta, 5, 5);
		test::AssertCacheContents(*cache.createView(), { 5, 6, 7, 8, 9 });
	}

	// endregion

	// region addedHistories / modifiedHistories / removedHistories

	namespace {
		std::set<MosaicId> CollectMosaicIds(const std::vector<const state::MosaicHistory*>& histories) {
			std::set<MosaicId> ids;
			for (const auto* pHistory : histories)
				ids.insert(pHistory->id());

			return ids;
		}

		std::set<MosaicId> ToSet(const std::vector<MosaicId>& ids) {
			return std::set<MosaicId>(ids.cbegin(), ids.cend());
		}

		void AssertMarkedHistories(
				const MosaicCacheDelta& delta,
				const std::set<MosaicId>& expectedAddedHistories,
				const std::set<MosaicId>& expectedModifiedHistories,
				const std::set<MosaicId>& expectedRemovedHistories) {
			// note that std::set is used because order of returned histories is not important
			EXPECT_EQ(expectedAddedHistories, CollectMosaicIds(delta.addedMosaicHistories()));
			EXPECT_EQ(expectedModifiedHistories, CollectMosaicIds(delta.modifiedMosaicHistories()));
			EXPECT_EQ(expectedRemovedHistories, ToSet(delta.removedMosaicHistories()));
		}
	}

	TEST(TEST_CLASS, InitiallyNoHistoriesAreMarkedAsAddedOrModifiedOrRemoved) {
		// Arrange:
		MosaicCache cache;
		auto delta = cache.createDelta();

		// Assert:
		AssertMarkedHistories(*delta, {}, {}, {});
	}

	TEST(TEST_CLASS, AddedHistoriesAreMarkedAsAdded) {
		// Arrange:
		MosaicCache cache;
		auto delta = cache.createDelta();

		// Act:
		delta->insert(test::CreateMosaicEntry(MosaicId(123), Amount()));

		// Assert:
		AssertMarkedHistories(*delta, { MosaicId(123) }, {}, {});
	}

	TEST(TEST_CLASS, ModifiedHistoriesAreMarkedAsModified) {
		// Arrange:
		MosaicCache cache;
		auto delta = cache.createDelta();
		delta->insert(test::CreateMosaicEntry(MosaicId(123), Amount()));
		cache.commit();

		// Act:
		delta->insert(test::CreateMosaicEntry(MosaicId(123), Amount()));

		// Assert:
		AssertMarkedHistories(*delta, {}, { MosaicId(123) }, {});
	}

	TEST(TEST_CLASS, RemovedHistoriesAreMarkedAsRemoved) {
		// Arrange:
		MosaicCache cache;
		auto delta = cache.createDelta();
		delta->insert(test::CreateMosaicEntry(MosaicId(123), Amount()));
		cache.commit();

		// Act:
		delta->remove(MosaicId(123));

		// Assert:
		AssertMarkedHistories(*delta, {}, {}, { MosaicId(123) });
	}

	TEST(TEST_CLASS, MultipleMarkedHistoriesCanBeTracked) {
		MosaicCache cache;
		auto delta = cache.createDelta();
		for (auto i = 100u; i < 110; ++i)
			delta->insert(test::CreateMosaicEntry(MosaicId(i), Amount()));

		cache.commit();

		// Act:
		// - add two
		delta->insert(test::CreateMosaicEntry(MosaicId(123), Amount()));
		delta->insert(test::CreateMosaicEntry(MosaicId(128), Amount()));

		// - modify three
		delta->insert(test::CreateMosaicEntry(MosaicId(105), Amount()));
		delta->insert(test::CreateMosaicEntry(MosaicId(107), Amount()));
		delta->insert(test::CreateMosaicEntry(MosaicId(108), Amount()));

		// - remove four
		delta->remove(MosaicId(100));
		delta->remove(MosaicId(101));
		delta->remove(MosaicId(104));
		delta->remove(MosaicId(106));

		// Assert:
		AssertMarkedHistories(
				*delta,
				{ MosaicId(123), MosaicId(128) },
				{ MosaicId(105), MosaicId(107), MosaicId(108) },
				{ MosaicId(100), MosaicId(101), MosaicId(104), MosaicId(106) });
	}

	// endregion

	// region general cache tests

	namespace {
		struct MosaicCacheEntityTraits {
		public:
			using KeyType = MosaicId;

			static auto CreateEntity(size_t id) {
				return test::CreateMosaicEntry(MosaicId(id), Amount(id));
			}

			static auto ToKey(const state::MosaicEntry& entry) {
				return entry.mosaicId();
			}

			static auto ToKey(const std::pair<MosaicId, state::MosaicHistory>& pair) {
				return pair.first;
			}
		};

		struct MosaicCacheTraits {
		public:
			using EntityTraits = MosaicCacheEntityTraits;

		public:
			template<typename TAction>
			static void RunEmptyCacheTest(TAction action) {
				// Arrange:
				MosaicCache cache;

				// Act:
				action(cache);
			}

			template<typename TAction>
			static void RunCacheTest(TAction action) {
				// Arrange:
				MosaicCache cache;
				auto entities = InsertMultiple(cache, { 1, 4, 9 });

				// Act:
				action(cache, entities);
			}

			static std::vector<MosaicId> InsertMultiple(MosaicCache& cache, std::initializer_list<size_t> ids) {
				auto delta = cache.createDelta();
				std::vector<MosaicId> entities;
				for (auto id : ids) {
					auto entry = EntityTraits::CreateEntity(id);
					delta->insert(entry);
					entities.push_back(EntityTraits::ToKey(entry));
				}

				cache.commit();
				return entities;
			}

		public:
			static void Insert(MosaicCacheDelta& delta, const state::MosaicEntry& entry) {
				delta.insert(entry);
			}

			static void Remove(MosaicCacheDelta& delta, const state::MosaicEntry& entry) {
				delta.remove(EntityTraits::ToKey(entry));
			}
		};
	}

	DEFINE_CACHE_CONTENTS_TESTS(TEST_CLASS, MosaicCacheTraits)
	DEFINE_CACHE_ITERATION_TESTS(TEST_CLASS, MosaicCacheTraits, Unordered)
	DEFINE_CACHE_SYNC_TESTS(TEST_CLASS, MosaicCacheTraits)

	// endregion
}}
