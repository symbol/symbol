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

#include "src/storages/MongoAccountRestrictionCacheStorage.h"
#include "mongo/src/mappers/MapperUtils.h"
#include "plugins/txes/restriction_account/src/state/AccountRestrictions.h"
#include "mongo/tests/test/MongoFlatCacheStorageTests.h"
#include "mongo/tests/test/MongoTestUtils.h"
#include "plugins/txes/restriction_account/tests/test/AccountRestrictionCacheTestUtils.h"
#include "tests/test/AccountRestrictionsMapperTestUtils.h"
#include "tests/TestHarness.h"

using namespace bsoncxx::builder::stream;

namespace catapult { namespace mongo { namespace plugins {

#define TEST_CLASS MongoAccountRestrictionCacheStorageTests

	namespace {
		constexpr auto Action_Add = model::AccountRestrictionModificationAction::Add;

		void InsertRandom(state::AccountRestriction& restriction, size_t count) {
			for (auto i = 0u; i < count; ++i)
				restriction.allow({ Action_Add, test::GenerateRandomVector(Address::Size) });
		}

		struct AccountRestrictionCacheTraits {
			using CacheType = cache::AccountRestrictionCache;
			using ModelType = state::AccountRestrictions;

			static constexpr auto Collection_Name = "accountRestrictions";
			static constexpr auto Primary_Document_Name = "accountRestrictions";
			static constexpr auto Network_Id = static_cast<model::NetworkIdentifier>(0x5A);
			static constexpr auto CreateCacheStorage = CreateMongoAccountRestrictionCacheStorage;

			static cache::CatapultCache CreateCache() {
				return test::AccountRestrictionCacheFactory::Create();
			}

			static ModelType GenerateRandomElement(uint32_t id) {
				Address address{};
				std::memcpy(address.data(), &id, sizeof(id));

				state::AccountRestrictions restrictions(address);

				auto& restriction = restrictions.restriction(model::AccountRestrictionFlags::Address);
				InsertRandom(restriction, std::max<size_t>(1, test::RandomByte() & 0x0F));
				return restrictions;
			}

			static void Add(cache::CatapultCacheDelta& delta, const ModelType& restrictions) {
				auto& restrictionCacheDelta = delta.sub<cache::AccountRestrictionCache>();
				restrictionCacheDelta.insert(restrictions);
			}

			static void Remove(cache::CatapultCacheDelta& delta, const ModelType& restrictions) {
				auto& restrictionCacheDelta = delta.sub<cache::AccountRestrictionCache>();
				restrictionCacheDelta.remove(restrictions.address());
			}

			static void Mutate(cache::CatapultCacheDelta& delta, ModelType& restrictions) {
				// update expected
				auto value = test::GenerateRandomVector(Address::Size);
				auto& restriction = restrictions.restriction(model::AccountRestrictionFlags::Address);
				restriction.allow({ Action_Add, value });

				// update cache
				auto& restrictionCacheDelta = delta.sub<cache::AccountRestrictionCache>();
				auto& restrictionsFromCache = restrictionCacheDelta.find(restrictions.address()).get();
				auto& restrictionFromCache = restrictionsFromCache.restriction(model::AccountRestrictionFlags::Address);
				restrictionFromCache.allow({ Action_Add, value });
			}

			static auto GetFindFilter(const ModelType& restrictions) {
				return document()
						<< std::string(Primary_Document_Name) + ".address" << mappers::ToBinary(restrictions.address())
						<< finalize;
			}

			static void AssertEqual(const ModelType& restrictions, const bsoncxx::document::view& view) {
				test::AssertEqualAccountRestrictionsData(restrictions, view[Primary_Document_Name].get_document().view());
			}
		};
	}

	DEFINE_FLAT_CACHE_STORAGE_TESTS(AccountRestrictionCacheTraits,)
}}}
