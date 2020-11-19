/**
*** Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
*** Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
*** All rights reserved.
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

#include "src/cache/AccountRestrictionCache.h"
#include "tests/test/cache/CacheBasicTests.h"
#include "tests/test/cache/CacheMixinsTests.h"
#include "tests/test/cache/DeltaElementsMixinTests.h"

namespace catapult { namespace cache {

#define TEST_CLASS AccountRestrictionCacheTests

	// region mixin traits based tests

	namespace {
		struct AccountRestrictionCacheMixinTraits {
			class CacheType : public AccountRestrictionCache {
			public:
				CacheType() : AccountRestrictionCache(CacheConfiguration(), model::NetworkIdentifier::Zero)
				{}
			};

			using IdType = Address;
			using ValueType = state::AccountRestrictions;

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
				return state::AccountRestrictions(MakeId(id));
			}
		};

		struct AccountRestrictionCacheDeltaModificationPolicy : public test:: DeltaInsertModificationPolicy {
			static void Modify(AccountRestrictionCacheDelta& delta, const state::AccountRestrictions& restrictions) {
				auto& restrictionsFromCache = delta.find(restrictions.address()).get();
				auto& restriction = restrictionsFromCache.restriction(model::AccountRestrictionFlags::Address);
				restriction.allow({ model::AccountRestrictionModificationAction::Add, test::GenerateRandomVector(Address::Size) });
			}
		};
	}

	DEFINE_CACHE_CONTAINS_TESTS(AccountRestrictionCacheMixinTraits, ViewAccessor, _View)
	DEFINE_CACHE_CONTAINS_TESTS(AccountRestrictionCacheMixinTraits, DeltaAccessor, _Delta)

	DEFINE_CACHE_ITERATION_TESTS(AccountRestrictionCacheMixinTraits, ViewAccessor, _View)

	DEFINE_CACHE_ACCESSOR_TESTS(AccountRestrictionCacheMixinTraits, ViewAccessor, MutableAccessor, _ViewMutable)
	DEFINE_CACHE_ACCESSOR_TESTS(AccountRestrictionCacheMixinTraits, ViewAccessor, ConstAccessor, _ViewConst)
	DEFINE_CACHE_ACCESSOR_TESTS(AccountRestrictionCacheMixinTraits, DeltaAccessor, MutableAccessor, _DeltaMutable)
	DEFINE_CACHE_ACCESSOR_TESTS(AccountRestrictionCacheMixinTraits, DeltaAccessor, ConstAccessor, _DeltaConst)

	DEFINE_CACHE_MUTATION_TESTS(AccountRestrictionCacheMixinTraits, DeltaAccessor, _Delta)

	DEFINE_DELTA_ELEMENTS_MIXIN_CUSTOM_TESTS(AccountRestrictionCacheMixinTraits, AccountRestrictionCacheDeltaModificationPolicy, _Delta)

	DEFINE_CACHE_BASIC_TESTS(AccountRestrictionCacheMixinTraits,)

	// endregion

	// region custom tests

	TEST(TEST_CLASS, CacheWrappersExposeNetworkIdentifier) {
		// Arrange:
		auto networkIdentifier = static_cast<model::NetworkIdentifier>(18);
		AccountRestrictionCache cache(CacheConfiguration(), networkIdentifier);

		// Act + Assert:
		EXPECT_EQ(networkIdentifier, cache.createView()->networkIdentifier());
		EXPECT_EQ(networkIdentifier, cache.createDelta()->networkIdentifier());
		EXPECT_EQ(networkIdentifier, cache.createDetachedDelta().tryLock()->networkIdentifier());
	}

	// endregion
}}
