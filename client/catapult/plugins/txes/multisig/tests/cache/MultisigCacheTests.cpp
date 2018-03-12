#include "src/cache/MultisigCache.h"
#include "tests/test/MultisigTestUtils.h"
#include "tests/test/cache/CacheBasicTests.h"
#include "tests/test/cache/CacheMixinsTests.h"
#include "tests/test/cache/DeltaElementsMixinTests.h"

namespace catapult { namespace cache {

#define TEST_CLASS MultisigCacheTests

	// region mixin traits based tests

	namespace {
		struct MultisigCacheMixinTraits {
			using CacheType = MultisigCache;
			using IdType = Key;
			using ValueType = state::MultisigEntry;

			static uint8_t GetRawId(const IdType& id) {
				return id[0];
			}

			static IdType GetId(const ValueType& entry) {
				return entry.key();
			}

			static IdType MakeId(uint8_t id) {
				return IdType{ { id } };
			}

			static ValueType CreateWithId(uint8_t id) {
				return state::MultisigEntry(MakeId(id));
			}
		};

		struct MultisigEntryModificationPolicy {
			static void Modify(MultisigCacheDelta& delta, const state::MultisigEntry& entry) {
				auto& entryFromCache = delta.get(entry.key());
				entryFromCache.cosignatories().insert(test::GenerateRandomData<Key_Size>());
			}
		};
	}

	DEFINE_CACHE_CONTAINS_TESTS(MultisigCacheMixinTraits, ViewAccessor, _View);
	DEFINE_CACHE_CONTAINS_TESTS(MultisigCacheMixinTraits, DeltaAccessor, _Delta);

	DEFINE_CACHE_ITERATION_TESTS(MultisigCacheMixinTraits, ViewAccessor, _View);

	DEFINE_CACHE_ACCESSOR_TESTS(MultisigCacheMixinTraits, ViewAccessor, MutableAccessor, _ViewMutable);
	DEFINE_CACHE_ACCESSOR_TESTS(MultisigCacheMixinTraits, ViewAccessor, ConstAccessor, _ViewConst);
	DEFINE_CACHE_ACCESSOR_TESTS(MultisigCacheMixinTraits, DeltaAccessor, MutableAccessor, _DeltaMutable);
	DEFINE_CACHE_ACCESSOR_TESTS(MultisigCacheMixinTraits, DeltaAccessor, ConstAccessor, _DeltaConst);

	DEFINE_CACHE_MUTATION_TESTS(MultisigCacheMixinTraits, DeltaAccessor, _Delta);

	DEFINE_DELTA_ELEMENTS_MIXIN_CUSTOM_TESTS(MultisigCacheMixinTraits, MultisigEntryModificationPolicy, _Delta);

	DEFINE_CACHE_BASIC_TESTS(MultisigCacheMixinTraits,);

	// endregion

	// region custom tests

	TEST(TEST_CLASS, LinksCanBeAddedLater) {
		// Arrange:
		MultisigCache cache;
		auto keys = test::GenerateKeys(3);

		// - insert single account key
		{

			auto delta = cache.createDelta();
			delta->insert(state::MultisigEntry(keys[0]));
			cache.commit();
		}

		// Sanity:
		EXPECT_EQ(1u, cache.createView()->size());

		// Act: add links
		{
			auto delta = cache.createDelta();
			auto& entry = delta->get(keys[0]);
			entry.cosignatories().insert(keys[1]);
			entry.multisigAccounts().insert(keys[2]);
			cache.commit();
		}

		// Assert:
		auto view = cache.createView();
		const auto& entry = view->get(keys[0]);
		EXPECT_EQ(utils::KeySet({ keys[1] }), entry.cosignatories());
		EXPECT_EQ(utils::KeySet({ keys[2] }), entry.multisigAccounts());
	}

	// endregion
}}
