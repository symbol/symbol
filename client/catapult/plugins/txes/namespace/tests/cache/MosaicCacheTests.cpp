#include "src/cache/MosaicCache.h"
#include "tests/test/MosaicCacheTestUtils.h"
#include "tests/test/MosaicTestUtils.h"
#include "tests/test/cache/CacheBasicTests.h"
#include "tests/test/cache/CacheMixinsTests.h"
#include "tests/test/cache/CachePruneTests.h"
#include "tests/test/cache/DeltaElementsMixinTests.h"
#include "tests/TestHarness.h"

namespace catapult { namespace cache {

#define TEST_CLASS MosaicCacheTests

	// region mixin traits based tests

	namespace {
		struct MosaicCacheMixinTraits {
			using CacheType = MosaicCache;
			using IdType = MosaicId;
			using ValueType = state::MosaicHistory;

			static uint8_t GetRawId(const IdType& id) {
				return static_cast<uint8_t>(id.unwrap());
			}

			static IdType GetId(const ValueType& history) {
				return history.id();
			}

			// additional overload required because of value mutation during access
			static IdType GetId(const state::MosaicEntry& entry) {
				return MakeId(entry.definition().owner()[0] / 2);
			}

			static IdType MakeId(uint8_t id) {
				return IdType(id);
			}

			static state::MosaicEntry CreateWithId(uint8_t id) {
				auto key = Key{ { static_cast<uint8_t>(id * 2) }};
				auto definition = state::MosaicDefinition(Height(), key, model::MosaicProperties::FromValues({}));
				return state::MosaicEntry(NamespaceId(111), MakeId(id), definition);
			}

			static state::MosaicEntry CreateWithIdAndExpiration(uint8_t id, Height height) {
				// simulate behavior of lock info cache activation (so expiration is at specified height)
				auto properties = model::MosaicProperties::FromValues({ { 0, 0, height.unwrap() - 1 } });
				auto definition = state::MosaicDefinition(Height(1), Key(), properties);
				return state::MosaicEntry(NamespaceId(123), MakeId(id), definition);
			}
		};
	}

	DEFINE_CACHE_CONTAINS_TESTS(MosaicCacheMixinTraits, ViewAccessor, _View);
	DEFINE_CACHE_CONTAINS_TESTS(MosaicCacheMixinTraits, DeltaAccessor, _Delta);

	DEFINE_CACHE_ITERATION_TESTS(MosaicCacheMixinTraits, ViewAccessor, _View);

	DEFINE_CACHE_ACCESSOR_TESTS(MosaicCacheMixinTraits, ViewAccessor, MutableAccessor, _ViewMutable);
	DEFINE_CACHE_ACCESSOR_TESTS(MosaicCacheMixinTraits, ViewAccessor, ConstAccessor, _ViewConst);
	DEFINE_CACHE_ACCESSOR_TESTS(MosaicCacheMixinTraits, DeltaAccessor, MutableAccessor, _DeltaMutable);
	DEFINE_CACHE_ACCESSOR_TESTS(MosaicCacheMixinTraits, DeltaAccessor, ConstAccessor, _DeltaConst);

	DEFINE_ACTIVE_PREDICATE_TESTS(MosaicCacheMixinTraits, ViewAccessor, _View);
	DEFINE_ACTIVE_PREDICATE_TESTS(MosaicCacheMixinTraits, DeltaAccessor, _Delta);

	DEFINE_DELTA_ELEMENTS_MIXIN_TESTS(MosaicCacheMixinTraits, _Delta);

	DEFINE_CACHE_BASIC_TESTS(MosaicCacheMixinTraits,);

	// endregion

	// *** custom tests ***

	namespace {
		model::MosaicProperties CreatePropertiesWithDuration(uint64_t duration) {
			return test::CreateMosaicPropertiesWithDuration(BlockDuration(duration));
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

		// Act + Assert: add additional history with an inconsistent namespace
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

		// Act + Assert:
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

	DEFINE_CACHE_PRUNE_TESTS(MosaicCacheMixinTraits,);

	TEST(TEST_CLASS, PruneRemovesExpiredMosaicsWhenHistoryDepthIsNotOneAndLatestExtendsDuration) {
		// Arrange:
		test::CachePruneTests<MosaicCacheMixinTraits>::RunCustomPruneTest([](auto& delta, auto pruneAndCommit) {
			// - extend the duration of mosaic id 5
			delta.insert(CreateMosaicEntry(MosaicId(5), Height(100), 10, Amount(333)));

			// Sanity:
			test::AssertCacheSizes(delta, 10, 11);

			// Act: prune mosaics with id 1 - 7 (the latest 5 entry was not pruned)
			pruneAndCommit(Height(70));

			// Assert:
			test::AssertCacheSizes(delta, 4, 4);
			test::AssertCacheContents(delta, { 5, 8, 9, 10 });
		});
	}

	TEST(TEST_CLASS, PruneRemovesExpiredMosaicsWhenHistoryDepthIsNotOneAndLatestShortensDuration) {
		// Arrange:
		test::CachePruneTests<MosaicCacheMixinTraits>::RunCustomPruneTest([](auto& delta, auto pruneAndCommit) {
			// - shorten the duration of mosaic id 9
			delta.insert(CreateMosaicEntry(MosaicId(9), Height(50), 10, Amount(333)));

			// Sanity:
			test::AssertCacheSizes(delta, 10, 11);

			// Act: prune mosaic with id 0 - 6 and 9 (the oldest 9 entry is pruned even though it is active)
			pruneAndCommit(Height(70));

			// Assert:
			test::AssertCacheSizes(delta, 2, 2);
			test::AssertCacheContents(delta, { 8, 10 });
		});
	}

	// endregion
}}
