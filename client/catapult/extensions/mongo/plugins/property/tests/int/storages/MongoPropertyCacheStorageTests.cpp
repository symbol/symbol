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

#include "src/storages/MongoPropertyCacheStorage.h"
#include "mongo/src/mappers/MapperUtils.h"
#include "plugins/txes/property/src/state/AccountProperties.h"
#include "mongo/tests/test/MongoFlatCacheStorageTests.h"
#include "mongo/tests/test/MongoTestUtils.h"
#include "plugins/txes/property/tests/test/PropertyCacheTestUtils.h"
#include "tests/test/AccountPropertiesMapperTestUtils.h"
#include "tests/TestHarness.h"

using namespace bsoncxx::builder::stream;

namespace catapult { namespace mongo { namespace plugins {

#define TEST_CLASS MongoPropertyCacheStorageTests

	namespace {
		constexpr auto Modification_Type_Add = model::PropertyModificationType::Add;

		void InsertRandom(state::AccountProperty& property, size_t count) {
			for (auto i = 0u; i < count; ++i)
				property.allow({ Modification_Type_Add, test::GenerateRandomVector(Address_Decoded_Size) });
		}

		struct PropertyCacheTraits {
			using CacheType = cache::PropertyCache;
			using ModelType = state::AccountProperties;

			static constexpr auto Collection_Name = "accountProperties";
			static constexpr auto Network_Id = static_cast<model::NetworkIdentifier>(0x5A);
			static constexpr auto CreateCacheStorage = CreateMongoPropertyCacheStorage;

			static cache::CatapultCache CreateCache() {
				return test::PropertyCacheFactory::Create();
			}

			static ModelType GenerateRandomElement(uint32_t id) {
				Address address{};
				std::memcpy(address.data(), &id, sizeof(id));

				state::AccountProperties accountProperties(address);

				auto& property = accountProperties.property(model::PropertyType::Address);
				InsertRandom(property, std::max<size_t>(1, test::RandomByte() & 0x0F));
				return accountProperties;
			}

			static void Add(cache::CatapultCacheDelta& delta, const ModelType& accountProperties) {
				auto& propertyCacheDelta = delta.sub<cache::PropertyCache>();
				propertyCacheDelta.insert(accountProperties);
			}

			static void Remove(cache::CatapultCacheDelta& delta, const ModelType& accountProperties) {
				auto& propertyCacheDelta = delta.sub<cache::PropertyCache>();
				propertyCacheDelta.remove(accountProperties.address());
			}

			static void Mutate(cache::CatapultCacheDelta& delta, ModelType& accountProperties) {
				// update expected
				auto value = test::GenerateRandomVector(Address_Decoded_Size);
				auto& property = accountProperties.property(model::PropertyType::Address);
				property.allow({ Modification_Type_Add, value });

				// update cache
				auto& propertyCacheDelta = delta.sub<cache::PropertyCache>();
				auto& accountPropertiesFromCache = propertyCacheDelta.find(accountProperties.address()).get();
				auto& propertyFromCache = accountPropertiesFromCache.property(model::PropertyType::Address);
				propertyFromCache.allow({ Modification_Type_Add, value });
			}

			static auto GetFindFilter(const ModelType& accountProperties) {
				return document() << "accountProperties.address" << mappers::ToBinary(accountProperties.address()) << finalize;
			}

			static void AssertEqual(const ModelType& accountProperties, const bsoncxx::document::view& view) {
				test::AssertEqualAccountPropertiesData(accountProperties, view["accountProperties"].get_document().view());
			}
		};
	}

	DEFINE_FLAT_CACHE_STORAGE_TESTS(PropertyCacheTraits,)
}}}
