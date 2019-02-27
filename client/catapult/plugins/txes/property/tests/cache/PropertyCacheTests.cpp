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

#include "src/cache/PropertyCache.h"
#include "tests/test/cache/CacheBasicTests.h"
#include "tests/test/cache/CacheMixinsTests.h"
#include "tests/test/cache/DeltaElementsMixinTests.h"

namespace catapult { namespace cache {

#define TEST_CLASS PropertyCacheTests

	// region mixin traits based tests

	namespace {
		struct PropertyCacheMixinTraits {
			class CacheType : public PropertyCache {
			public:
				CacheType() : PropertyCache(CacheConfiguration(), model::NetworkIdentifier::Zero)
				{}
			};

			using IdType = Address;
			using ValueType = state::AccountProperties;

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
				return state::AccountProperties(MakeId(id));
			}
		};

		struct PropertyEntryModificationPolicy {
			static void Modify(PropertyCacheDelta& delta, const state::AccountProperties& properties) {
				auto& propertiesFromCache = delta.find(properties.address()).get();
				auto& property = propertiesFromCache.property(model::PropertyType::Address);
				property.allow({ model::PropertyModificationType::Add, test::GenerateRandomVector(Address_Decoded_Size) });
			}
		};
	}

	DEFINE_CACHE_CONTAINS_TESTS(PropertyCacheMixinTraits, ViewAccessor, _View)
	DEFINE_CACHE_CONTAINS_TESTS(PropertyCacheMixinTraits, DeltaAccessor, _Delta)

	DEFINE_CACHE_ITERATION_TESTS(PropertyCacheMixinTraits, ViewAccessor, _View)

	DEFINE_CACHE_ACCESSOR_TESTS(PropertyCacheMixinTraits, ViewAccessor, MutableAccessor, _ViewMutable)
	DEFINE_CACHE_ACCESSOR_TESTS(PropertyCacheMixinTraits, ViewAccessor, ConstAccessor, _ViewConst)
	DEFINE_CACHE_ACCESSOR_TESTS(PropertyCacheMixinTraits, DeltaAccessor, MutableAccessor, _DeltaMutable)
	DEFINE_CACHE_ACCESSOR_TESTS(PropertyCacheMixinTraits, DeltaAccessor, ConstAccessor, _DeltaConst)

	DEFINE_CACHE_MUTATION_TESTS(PropertyCacheMixinTraits, DeltaAccessor, _Delta)

	DEFINE_DELTA_ELEMENTS_MIXIN_CUSTOM_TESTS(PropertyCacheMixinTraits, PropertyEntryModificationPolicy, _Delta)

	DEFINE_CACHE_BASIC_TESTS(PropertyCacheMixinTraits,)

	// endregion

	// region custom tests

	TEST(TEST_CLASS, CacheWrappersExposeNetworkIdentifier) {
		// Arrange:
		auto networkIdentifier = static_cast<model::NetworkIdentifier>(18);
		PropertyCache cache(CacheConfiguration(), networkIdentifier);

		// Act + Assert:
		EXPECT_EQ(networkIdentifier, cache.createView()->networkIdentifier());
		EXPECT_EQ(networkIdentifier, cache.createDelta()->networkIdentifier());
		EXPECT_EQ(networkIdentifier, cache.createDetachedDelta().tryLock()->networkIdentifier());
	}

	// endregion
}}
