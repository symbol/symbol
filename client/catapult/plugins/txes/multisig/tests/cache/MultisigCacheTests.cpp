#include "src/cache/MultisigCache.h"
#include "tests/test/MultisigTestUtils.h"
#include "tests/test/cache/CacheContentsTests.h"
#include "tests/test/cache/CacheIterationTests.h"
#include "tests/test/cache/CacheSynchronizationTests.h"
#include <map>

#define TEST_CLASS MultisigCacheTests

namespace catapult { namespace cache {

	namespace {
		using AccountKeys = std::vector<Key>;

		void AddAllAccountKeys(const AccountKeys& accountKeys, LockedCacheDelta<MultisigCacheDelta>& delta) {
			// add all accountKeys
			for (const auto& key : accountKeys)
				delta->insert(state::MultisigEntry(key));
		}
	}

	TEST(TEST_CLASS, CanAddSingleEntry) {
		// Arrange:
		MultisigCache cache;
		auto accountKeys = test::GenerateKeys(1);
		{
			// insert single account key
			auto delta = cache.createDelta();
			AddAllAccountKeys(accountKeys, delta);

			// Assert:
			test::AssertMultisigCacheContents(accountKeys, *delta);

			cache.commit();
		}

		// Assert:
		auto view = cache.createView();
		test::AssertMultisigCacheContents(accountKeys, *view);
	}

	namespace {
		struct ViewTraits {
			using ViewType = MultisigCacheView;

			template<typename TPopulate, typename TAssert>
			static void RunTest(TPopulate populate, TAssert assertResult) {
				// Arrange:
				auto accountKeys = test::GenerateKeys(10);
				MultisigCache cache;
				{
					auto delta = cache.createDelta();
					populate(accountKeys, delta);
					cache.commit();
				}

				// Act:
				auto view = cache.createView();
				assertResult(accountKeys, *view);
			}
		};

		struct DeltaTraits {
			using ViewType = MultisigCacheDelta;

			template<typename TPopulate, typename TAssert>
			static void RunTest(TPopulate populate, TAssert assertResult) {
				// Arrange:
				MultisigCache cache;
				auto delta = cache.createDelta();
				auto accountKeys = test::GenerateKeys(10);
				populate(accountKeys, delta);

				// Act:
				assertResult(accountKeys, *delta);
			}
		};

		struct CosignatoriesTraits {
			static auto& GetKeySet(state::MultisigEntry& entry) {
				return entry.cosignatories();
			}

			static auto& GetKeySet(const state::MultisigEntry& entry) {
				return entry.cosignatories();
			}
		};

		struct MultisigAccountsTraits {
			static auto& GetKeySet(state::MultisigEntry& entry) {
				return entry.multisigAccounts();
			}

			static auto& GetKeySet(const state::MultisigEntry& entry) {
				return entry.multisigAccounts();
			}
		};
	}

#define DELTA_VIEW_BASED_TEST(TEST_NAME) \
	template<typename TViewTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_View) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<ViewTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Delta) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<DeltaTraits>(); } \
	template<typename TViewTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

#define SET_BASED_TEST(TEST_NAME) \
	template<typename TSetTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_Cosignatories) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<CosignatoriesTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_MultisigAccounts) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<MultisigAccountsTraits>(); } \
	template<typename TSetTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

#define DELTA_VIEW_AND_SET_BASED_TEST(TEST_NAME) \
	template<typename TViewTraits, typename TSetTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_Cosignatories_View) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<ViewTraits, CosignatoriesTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Cosignatories_Delta) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<DeltaTraits, CosignatoriesTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_MultisigAccounts_View) { \
			TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<ViewTraits, MultisigAccountsTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_MultisigAccounts_Delta) { \
			TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<DeltaTraits, MultisigAccountsTraits>(); } \
	template<typename TViewTraits, typename TSetTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	DELTA_VIEW_BASED_TEST(CanAddEntries) {
		// Act:
		TViewTraits::RunTest(&AddAllAccountKeys, &test::AssertMultisigCacheContents<AccountKeys, typename TViewTraits::ViewType>);
	}

	DELTA_VIEW_BASED_TEST(GetReturnsKnownEntry) {
		// Act:
		TViewTraits::RunTest(&AddAllAccountKeys, [](const auto& accountKeys, const auto& view) {
			const auto& entry = view.get(accountKeys[5]);

			// Assert:
			EXPECT_EQ(accountKeys[5], entry.key());
		});
	}

	DELTA_VIEW_BASED_TEST(GetThrowsIfAccountKeyIsUnknown) {
		// Act:
		TViewTraits::RunTest(&AddAllAccountKeys, [](const auto&, const auto& view) {
			auto key = test::GenerateRandomData<Key_Size>();

			// Assert:
			EXPECT_THROW(view.get(key), catapult_invalid_argument);
		});
	}

	TEST(TEST_CLASS, GetConstAndNonConstOverloadsReturnReferencesToSameEntry) {
		// Act:
		DeltaTraits::RunTest(&AddAllAccountKeys, [](const auto& accountKeys, auto& view) {
			auto& mutableEntry = view.get(accountKeys[5]);
			auto& constEntry = const_cast<const MultisigCacheDelta&>(view).get(accountKeys[5]);

			// Assert:
			EXPECT_EQ(&mutableEntry, &constEntry);
		});
	}

	// region links

	namespace {
		template<typename TSetTraits>
		void AddLinks(const AccountKeys& accountKeys, LockedCacheDelta<MultisigCacheDelta>& delta) {
			for (auto mainAccountId : { 0u, 5u }) {
				delta->insert(state::MultisigEntry(accountKeys[mainAccountId]));
				auto& entry = delta->get(accountKeys[mainAccountId]);
				auto& keySet = TSetTraits::GetKeySet(entry);
				for (auto id : { 1u, 2u, 3u, 4u })
					keySet.insert(accountKeys[mainAccountId + id]);
			}
		}

		template<typename TView, typename TSetTraits>
		void VerifyLinks(const AccountKeys& accountKeys, const TView& view) {
			for (auto mainAccountId : { 0u, 5u }) {
				const auto& entry = view.get(accountKeys[mainAccountId]);
				const auto& keySet = TSetTraits::GetKeySet(entry);
				EXPECT_EQ(4u, keySet.size());
				for (auto id : { 1u, 2u, 3u, 4u }) {
					auto accountId = mainAccountId + id;
					EXPECT_TRUE(keySet.cend() != keySet.find(accountKeys[accountId])) << "missing account " << accountId;
				}
			}
		}
	}

	DELTA_VIEW_AND_SET_BASED_TEST(EntriesCanHaveLinks) {
		// Act:
		TViewTraits::RunTest(&AddLinks<TSetTraits>, &VerifyLinks<typename TViewTraits::ViewType, TSetTraits>);
	}

	SET_BASED_TEST(LinksCanBeAddedLater) {
		// Arrange:
		MultisigCache cache;
		auto accountKeys = test::GenerateKeys(2);

		// Act:
		{
			// insert single account key
			auto delta = cache.createDelta();
			delta->insert(state::MultisigEntry(accountKeys[0]));

			// Sanity:
			EXPECT_EQ(1u, delta->size());

			cache.commit();
		}

		{
			// add a link
			auto delta = cache.createDelta();
			auto& entry = delta->get(accountKeys[0]);
			auto& keySet = TSetTraits::GetKeySet(entry);
			keySet.insert(accountKeys[1]);

			// Sanity:
			EXPECT_EQ(1u, delta->size());

			cache.commit();
		}

		// Assert:
		auto view = cache.createView();
		const auto& entry = view->get(accountKeys[0]);
		const auto& keySet = TSetTraits::GetKeySet(entry);
		EXPECT_EQ(1u, keySet.size());
		EXPECT_TRUE(keySet.cend() != keySet.find(accountKeys[1]));
	}

	// endregion

	// region modifiedEntries / removedEntries

	namespace {
		utils::KeySet CollectKeys(const std::vector<const state::MultisigEntry*>& entries) {
			utils::KeySet keys;
			for (const auto* pEntry : entries)
				keys.insert(pEntry->key());

			return keys;
		}

		utils::KeySet ToSet(const std::vector<Key>& ids) {
			return utils::KeySet(ids.cbegin(), ids.cend());
		}

		void AssertMarkedEntries(
			const MultisigCacheDelta& delta,
			const utils::KeySet& expectedModifiedEntries,
			const utils::KeySet& expectedRemovedEntries) {
			// note that std::unordered_set is used because order of returned histories is not important
			EXPECT_EQ(expectedModifiedEntries, CollectKeys(delta.modifiedEntries()));
			EXPECT_EQ(expectedRemovedEntries, ToSet(delta.removedEntries()));
		}
	}

	TEST(TEST_CLASS, InitiallyNoEntriesAreMarkedAsModifiedOrRemoved) {
		// Arrange:
		MultisigCache cache;
		auto delta = cache.createDelta();

		// Assert:
		AssertMarkedEntries(*delta, {}, {});
	}

	TEST(TEST_CLASS, AddedEntriesAreMarkedAsModified) {
		// Arrange:
		MultisigCache cache;
		auto delta = cache.createDelta();
		auto key = test::GenerateRandomData<Key_Size>();

		// Act:
		delta->insert(state::MultisigEntry(key));

		// Assert:
		AssertMarkedEntries(*delta, { key }, {});
	}

	TEST(TEST_CLASS, ModifiedEntriesAreMarkedAsModified) {
		// Arrange:
		MultisigCache cache;
		auto delta = cache.createDelta();
		auto key = test::GenerateRandomData<Key_Size>();
		delta->insert(state::MultisigEntry(key));
		cache.commit();

		// Act:
		delta->insert(state::MultisigEntry(key));

		// Assert:
		AssertMarkedEntries(*delta, { key }, {});
	}

	TEST(TEST_CLASS, RemovedEntriesAreMarkedAsRemoved) {
		// Arrange:
		MultisigCache cache;
		auto delta = cache.createDelta();
		auto key = test::GenerateRandomData<Key_Size>();
		delta->insert(state::MultisigEntry(key));
		cache.commit();

		// Act:
		delta->remove(key);

		// Assert:
		AssertMarkedEntries(*delta, {}, { key });
	}

	TEST(TEST_CLASS, MultipleMarkedEntriesCanBeTracked) {
		MultisigCache cache;
		auto delta = cache.createDelta();
		std::vector<Key> keys;
		keys.push_back(test::GenerateRandomData<Key_Size>());
		keys.push_back(test::GenerateRandomData<Key_Size>());
		for (auto i = 0u; i < 7; ++i) {
			auto key = test::GenerateRandomData<Key_Size>();
			keys.push_back(key);
			delta->insert(state::MultisigEntry(key));
		}

		cache.commit();

		// Act:
		// - add two
		delta->insert(state::MultisigEntry(keys[0]));
		delta->insert(state::MultisigEntry(keys[1]));

		// - modify three
		delta->insert(state::MultisigEntry(keys[3]));
		delta->insert(state::MultisigEntry(keys[5]));
		delta->insert(state::MultisigEntry(keys[7]));

		// - remove four
		delta->remove(keys[2]);
		delta->remove(keys[4]);
		delta->remove(keys[6]);
		delta->remove(keys[8]);

		// Assert:
		AssertMarkedEntries(
				*delta,
				{ keys[0], keys[1], keys[3], keys[5], keys[7] },
				{ keys[2], keys[4], keys[6], keys[8] });
	}

	// endregion

	namespace {
		struct MultisigCacheEntityTraits {
		public:
			static auto CreateEntity(size_t id) {
				return state::MultisigEntry(Key{ { static_cast<uint8_t>(id) } });
			}

		public:
			// note cache tests expect result of ToKey to have type that is used by cache
			static auto ToKey(const state::MultisigEntry& entry) {
				return entry.key();
			}

			static auto ToKey(const std::pair<Key, state::MultisigEntry>& pair) {
				return pair.first;
			}
		};

		struct MultisigCacheTraits {
		public:
			using EntityTraits = MultisigCacheEntityTraits;

			template<typename TAction>
			static void RunEmptyCacheTest(TAction action) {
				// Arrange:
				MultisigCache cache;

				// Act:
				action(cache);
			}

			template<typename TAction>
			static void RunCacheTest(TAction action) {
				// Arrange:
				MultisigCache cache;
				auto entities = InsertMultiple(cache, { 1, 4, 9 });

				// Act:
				action(cache, entities);
			}

			static std::vector<Key> InsertMultiple(
					MultisigCache& cache,
					std::initializer_list<size_t> ids) {
				auto delta = cache.createDelta();
				std::vector<Key> keys;
				for (auto id : ids) {
					auto entry = EntityTraits::CreateEntity(id);
					delta->insert(entry);
					keys.push_back(entry.key());
				}

				cache.commit();
				return keys;
			}

		public:
			static void Insert(MultisigCacheDelta& delta, const state::MultisigEntry& entry) {
				delta.insert(entry);
			}

			static void Remove(MultisigCacheDelta& delta, const state::MultisigEntry& entry) {
				delta.remove(EntityTraits::ToKey(entry));
			}
		};
	}

	DEFINE_CACHE_CONTENTS_TESTS(TEST_CLASS, MultisigCacheTraits)
	DEFINE_CACHE_ITERATION_TESTS(TEST_CLASS, MultisigCacheTraits, Unordered)
	DEFINE_CACHE_SYNC_TESTS(TEST_CLASS, MultisigCacheTraits)
}}
