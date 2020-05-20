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
			class CacheType : public MultisigCache {
			public:
				CacheType() : MultisigCache(CacheConfiguration())
				{}
			};

			using IdType = Address;
			using ValueType = state::MultisigEntry;

			static uint8_t GetRawId(const IdType& id) {
				return id[0];
			}

			static IdType GetId(const ValueType& entry) {
				return entry.address();
			}

			static IdType MakeId(uint8_t id) {
				return IdType{ { id } };
			}

			static ValueType CreateWithId(uint8_t id) {
				return state::MultisigEntry(MakeId(id));
			}
		};

		struct MultisigCacheDeltaModificationPolicy : public test:: DeltaInsertModificationPolicy {
			static void Modify(MultisigCacheDelta& delta, const state::MultisigEntry& entry) {
				auto multisigIter = delta.find(entry.address());
				auto& entryFromCache = multisigIter.get();
				entryFromCache.cosignatoryAddresses().insert(test::GenerateRandomByteArray<Address>());
			}
		};
	}

	DEFINE_CACHE_CONTAINS_TESTS(MultisigCacheMixinTraits, ViewAccessor, _View)
	DEFINE_CACHE_CONTAINS_TESTS(MultisigCacheMixinTraits, DeltaAccessor, _Delta)

	DEFINE_CACHE_ITERATION_TESTS(MultisigCacheMixinTraits, ViewAccessor, _View)

	DEFINE_CACHE_ACCESSOR_TESTS(MultisigCacheMixinTraits, ViewAccessor, MutableAccessor, _ViewMutable)
	DEFINE_CACHE_ACCESSOR_TESTS(MultisigCacheMixinTraits, ViewAccessor, ConstAccessor, _ViewConst)
	DEFINE_CACHE_ACCESSOR_TESTS(MultisigCacheMixinTraits, DeltaAccessor, MutableAccessor, _DeltaMutable)
	DEFINE_CACHE_ACCESSOR_TESTS(MultisigCacheMixinTraits, DeltaAccessor, ConstAccessor, _DeltaConst)

	DEFINE_CACHE_MUTATION_TESTS(MultisigCacheMixinTraits, DeltaAccessor, _Delta)

	DEFINE_DELTA_ELEMENTS_MIXIN_CUSTOM_TESTS(MultisigCacheMixinTraits, MultisigCacheDeltaModificationPolicy, _Delta)

	DEFINE_CACHE_BASIC_TESTS(MultisigCacheMixinTraits,)

	// endregion

	// region custom tests

	TEST(TEST_CLASS, LinksCanBeAddedLater) {
		// Arrange:
		MultisigCacheMixinTraits::CacheType cache;
		auto addresses = test::GenerateRandomDataVector<Address>(3);

		// - insert single entry
		{

			auto delta = cache.createDelta();
			delta->insert(state::MultisigEntry(addresses[0]));
			cache.commit();
		}

		// Sanity:
		EXPECT_EQ(1u, cache.createView()->size());

		// Act: add links
		{
			auto delta = cache.createDelta();
			auto multisigIter = delta->find(addresses[0]);
			auto& entry = multisigIter.get();
			entry.cosignatoryAddresses().insert(addresses[1]);
			entry.multisigAddresses().insert(addresses[2]);
			cache.commit();
		}

		// Assert:
		auto view = cache.createView();
		auto multisigIter = view->find(addresses[0]);
		const auto& entry = multisigIter.get();
		EXPECT_EQ(state::SortedAddressSet({ addresses[1] }), entry.cosignatoryAddresses());
		EXPECT_EQ(state::SortedAddressSet({ addresses[2] }), entry.multisigAddresses());
	}

	// endregion
}}
