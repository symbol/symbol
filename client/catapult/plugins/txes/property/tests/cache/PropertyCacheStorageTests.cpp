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

#include "src/cache/PropertyCacheStorage.h"
#include "src/cache/PropertyCache.h"
#include "src/model/PropertyTypes.h"
#include "tests/test/AccountPropertiesTestUtils.h"
#include "tests/test/cache/CacheStorageTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace cache {

	namespace {
		struct PropertyCacheStorageTraits {
			using StorageType = PropertyCacheStorage;
			class CacheType : public PropertyCache {
			public:
				CacheType() : PropertyCache(CacheConfiguration(), static_cast<model::NetworkIdentifier>(12))
				{}
			};

			static auto CreateId(uint8_t id) {
				return Address{ { id } };
			}

			static auto CreateValue(const Address& address) {
				state::AccountProperties accountProperties(address);
				auto& property = accountProperties.property(model::PropertyType::Address);
				for (auto i = 0u; i < 3; ++i)
					property.allow({ model::PropertyModificationType::Add, test::GenerateRandomVector(Address_Decoded_Size) });

				return accountProperties;
			}

			static void AssertEqual(const state::AccountProperties& lhs, const state::AccountProperties& rhs) {
				test::AssertEqual(lhs, rhs);
			}
		};
	}

	DEFINE_BASIC_INSERT_REMOVE_CACHE_STORAGE_TESTS(PropertyCacheStorageTests, PropertyCacheStorageTraits)
}}
