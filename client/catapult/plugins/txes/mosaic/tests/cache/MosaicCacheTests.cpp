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

#include "src/cache/MosaicCache.h"
#include "tests/test/MosaicCacheTestUtils.h"
#include "tests/test/MosaicTestUtils.h"
#include "tests/test/cache/CacheBasicTests.h"
#include "tests/test/cache/CacheMixinsTests.h"
#include "tests/test/cache/DeltaElementsMixinTests.h"
#include "tests/TestHarness.h"

namespace catapult { namespace cache {

#define TEST_CLASS MosaicCacheTests

	// region mixin traits based tests

	namespace {
		struct MosaicCacheMixinTraits {
			class CacheType : public MosaicCache {
			public:
				CacheType() : MosaicCache(CacheConfiguration())
				{}
			};

			using IdType = MosaicId;
			using ValueType = state::MosaicEntry;

			static uint8_t GetRawId(const IdType& id) {
				return static_cast<uint8_t>(id.unwrap());
			}

			static IdType GetId(const ValueType& entry) {
				return entry.mosaicId();
			}

			static IdType MakeId(uint8_t id) {
				return IdType(id);
			}

			static ValueType CreateWithId(uint8_t id) {
				auto owner = Address{ { static_cast<uint8_t>(id * 2) }};
				auto definition = state::MosaicDefinition(Height(), owner, 3, model::MosaicProperties());
				return state::MosaicEntry(MakeId(id), definition);
			}

			static ValueType CreateWithIdAndExpiration(uint8_t id, Height height) {
				// simulate behavior of lock info cache activation (so expiration is at specified height)
				auto properties = test::CreateMosaicPropertiesWithDuration(BlockDuration(height.unwrap() - 1));
				auto definition = state::MosaicDefinition(Height(1), Address(), 3, properties);
				return state::MosaicEntry(MakeId(id), definition);
			}
		};

		struct MosaicCacheDeltaModificationPolicy : public test:: DeltaInsertModificationPolicy {
			static void Modify(MosaicCacheDelta& delta, const state::MosaicEntry& entry) {
				auto mosaicIter = delta.find(entry.mosaicId());
				auto& entryFromCache = mosaicIter.get();
				entryFromCache.increaseSupply(Amount(1));
			}
		};
	}

	DEFINE_CACHE_CONTAINS_TESTS(MosaicCacheMixinTraits, ViewAccessor, _View)
	DEFINE_CACHE_CONTAINS_TESTS(MosaicCacheMixinTraits, DeltaAccessor, _Delta)

	DEFINE_CACHE_ITERATION_TESTS(MosaicCacheMixinTraits, ViewAccessor, _View)

	DEFINE_CACHE_ACCESSOR_TESTS(MosaicCacheMixinTraits, ViewAccessor, MutableAccessor, _ViewMutable)
	DEFINE_CACHE_ACCESSOR_TESTS(MosaicCacheMixinTraits, ViewAccessor, ConstAccessor, _ViewConst)
	DEFINE_CACHE_ACCESSOR_TESTS(MosaicCacheMixinTraits, DeltaAccessor, MutableAccessor, _DeltaMutable)
	DEFINE_CACHE_ACCESSOR_TESTS(MosaicCacheMixinTraits, DeltaAccessor, ConstAccessor, _DeltaConst)

	DEFINE_CACHE_MUTATION_TESTS(MosaicCacheMixinTraits, DeltaAccessor, _Delta)

	DEFINE_ACTIVE_PREDICATE_TESTS(MosaicCacheMixinTraits, ViewAccessor, _View)
	DEFINE_ACTIVE_PREDICATE_TESTS(MosaicCacheMixinTraits, DeltaAccessor, _Delta)

	DEFINE_CACHE_TOUCH_TESTS(MosaicCacheMixinTraits, _Delta)

	DEFINE_DELTA_ELEMENTS_MIXIN_CUSTOM_TESTS(MosaicCacheMixinTraits, MosaicCacheDeltaModificationPolicy, _Delta)

	DEFINE_CACHE_BASIC_TESTS(MosaicCacheMixinTraits,)

	// endregion

	// region remove

	namespace {
		constexpr size_t Default_Cache_Size = 10;

		void PopulateCache(LockedCacheDelta<MosaicCacheDelta>& delta) {
			for (uint8_t i = 0; i < Default_Cache_Size; ++i)
				delta->insert(MosaicCacheMixinTraits::CreateWithIdAndExpiration(static_cast<uint8_t>(i + 1), Height(i + 1)));
		}
	}

	TEST(TEST_CLASS, CanRemoveEternalMosaic) {
		// Arrange: populate cache with some entries and insert eternal mosaic
		MosaicCacheMixinTraits::CacheType cache;
		{
			auto delta = cache.createDelta();
			PopulateCache(delta);
			delta->insert(MosaicCacheMixinTraits::CreateWithId(15));

			// Sanity:
			EXPECT_EQ(Default_Cache_Size + 1, delta->size());
			EXPECT_TRUE(delta->find(MosaicId(15)).get().definition().isEternal());

			cache.commit();
		}

		{
			// Act: remove mosaic with id 15
			auto delta = cache.createDelta();
			delta->remove(MosaicId(15));
			cache.commit();
		}

		// Assert:
		EXPECT_FALSE(cache.createView()->contains(MosaicId(15)));
	}

	TEST(TEST_CLASS, RemoveAndReinsertAddsNewEntryInHeightBasedMap) {
		// Arrange: populate cache with some entries and insert mosaic with id 15 and expiry height 123
		MosaicCacheMixinTraits::CacheType cache;
		{
			auto delta = cache.createDelta();
			PopulateCache(delta);
			delta->insert(MosaicCacheMixinTraits::CreateWithIdAndExpiration(15, Height(123)));

			// Sanity:
			EXPECT_EQ(Default_Cache_Size + 1, delta->size());
			EXPECT_CONTAINS(delta->touch(Height(123)), MosaicId(15));

			cache.commit();
		}

		{
			// Act: remove mosaic with id 15
			auto delta = cache.createDelta();
			delta->remove(MosaicId(15));
			cache.commit();
		}

		// Act: reinsert mosaic with id 15 and expiry height 234
		auto delta = cache.createDelta();
		delta->insert(MosaicCacheMixinTraits::CreateWithIdAndExpiration(15, Height(234)));

		// Assert: height based entry at height 234 was added
		auto deactivatingIds1 = delta->touch(Height(123));
		EXPECT_TRUE(deactivatingIds1.empty());

		auto deactivatingIds2 = delta->touch(Height(234));
		EXPECT_CONTAINS(deactivatingIds2, MosaicId(15));
	}

	// endregion
}}
