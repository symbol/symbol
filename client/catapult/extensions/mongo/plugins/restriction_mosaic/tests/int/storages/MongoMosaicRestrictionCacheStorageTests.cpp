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

#include "src/storages/MongoMosaicRestrictionCacheStorage.h"
#include "mongo/src/mappers/MapperUtils.h"
#include "plugins/txes/restriction_mosaic/src/cache/MosaicRestrictionCache.h"
#include "mongo/tests/test/MongoFlatCacheStorageTests.h"
#include "mongo/tests/test/MongoTestUtils.h"
#include "plugins/txes/restriction_mosaic/tests/test/MosaicRestrictionCacheTestUtils.h"
#include "tests/test/MosaicRestrictionEntryMapperTestUtils.h"
#include "tests/TestHarness.h"

using namespace bsoncxx::builder::stream;

namespace catapult { namespace mongo { namespace plugins {

#define TEST_CLASS MongoMosaicRestrictionCacheStorageTests

	namespace {
		struct BasicMosaicRestrictionCacheTraits {
		public:
			using CacheType = cache::MosaicRestrictionCache;
			using ModelType = state::MosaicRestrictionEntry;

			static constexpr auto Collection_Name = "mosaicRestrictions";
			static constexpr auto Primary_Document_Name = "mosaicRestrictionEntry";
			static constexpr auto Network_Id = static_cast<model::NetworkIdentifier>(0x5A);
			static constexpr auto CreateCacheStorage = CreateMongoMosaicRestrictionCacheStorage;

		public:
			static cache::CatapultCache CreateCache() {
				return test::MosaicRestrictionCacheFactory::Create();
			}

			static void Add(cache::CatapultCacheDelta& delta, const ModelType& restrictionEntry) {
				auto& restrictionCacheDelta = delta.sub<cache::MosaicRestrictionCache>();
				restrictionCacheDelta.insert(restrictionEntry);
			}

			static void Remove(cache::CatapultCacheDelta& delta, const ModelType& restrictionEntry) {
				auto& restrictionCacheDelta = delta.sub<cache::MosaicRestrictionCache>();
				restrictionCacheDelta.remove(restrictionEntry.uniqueKey());
			}

			static auto GetFindFilter(const ModelType& restrictionEntry) {
				return document()
						<< std::string(Primary_Document_Name) + ".compositeHash" << mappers::ToBinary(restrictionEntry.uniqueKey())
						<< finalize;
			}
		};

		struct MosaicAddressRestrictionCacheTraits : public BasicMosaicRestrictionCacheTraits {
		public:
			static ModelType GenerateRandomElement(uint32_t id) {
				MosaicId mosaicId(id);
				Address address;
				std::memcpy(address.data(), &id, sizeof(id));

				auto restrictionEntry = state::MosaicRestrictionEntry(state::MosaicAddressRestriction(mosaicId, address));
				restrictionEntry.asAddressRestriction().set(test::Random(), test::Random());
				return restrictionEntry;
			}

			static void Mutate(cache::CatapultCacheDelta& delta, ModelType& restrictionEntry) {
				// update expected
				auto key = test::Random();
				auto value = test::Random();
				restrictionEntry.asAddressRestriction().set(key, value);

				// update cache
				auto& restrictionCacheDelta = delta.sub<cache::MosaicRestrictionCache>();
				auto& restrictionEntryFromCache = restrictionCacheDelta.find(restrictionEntry.uniqueKey()).get();
				restrictionEntryFromCache.asAddressRestriction().set(key, value);
			}

			static void AssertEqual(const ModelType& restrictionEntry, const bsoncxx::document::view& view) {
				auto dbRestrictionEntry = view[Primary_Document_Name].get_document().view();
				test::MosaicAddressRestrictionTestTraits::AssertEqualRestriction(restrictionEntry, dbRestrictionEntry);
			}
		};

		struct MosaicGlobalRestrictionCacheTraits : public BasicMosaicRestrictionCacheTraits {
		public:
			static ModelType GenerateRandomElement(uint32_t id) {
				MosaicId mosaicId(id);

				auto restrictionEntry = state::MosaicRestrictionEntry(state::MosaicGlobalRestriction(mosaicId));
				restrictionEntry.asGlobalRestriction().set(test::Random(), CreateRandomRestrictionRule());
				return restrictionEntry;
			}

			static void Mutate(cache::CatapultCacheDelta& delta, ModelType& restrictionEntry) {
				// update expected
				auto key = test::Random();
				auto rule = CreateRandomRestrictionRule();
				restrictionEntry.asGlobalRestriction().set(key, rule);

				// update cache
				auto& restrictionCacheDelta = delta.sub<cache::MosaicRestrictionCache>();
				auto& restrictionEntryFromCache = restrictionCacheDelta.find(restrictionEntry.uniqueKey()).get();
				restrictionEntryFromCache.asGlobalRestriction().set(key, rule);
			}

			static void AssertEqual(const ModelType& restrictionEntry, const bsoncxx::document::view& view) {
				auto dbRestrictionEntry = view[Primary_Document_Name].get_document().view();
				test::MosaicGlobalRestrictionTestTraits::AssertEqualRestriction(restrictionEntry, dbRestrictionEntry);
			}

		private:
			static state::MosaicGlobalRestriction::RestrictionRule CreateRandomRestrictionRule() {
				auto referenceMosaicId = test::GenerateRandomValue<MosaicId>();
				auto restrictionValue = test::Random();
				auto restrictionType = static_cast<model::MosaicRestrictionType>(test::RandomByte() | 1);
				return state::MosaicGlobalRestriction::RestrictionRule{ referenceMosaicId, restrictionValue, restrictionType };
			}
		};
	}

	DEFINE_FLAT_CACHE_STORAGE_TESTS(MosaicAddressRestrictionCacheTraits, _Address)
	DEFINE_FLAT_CACHE_STORAGE_TESTS(MosaicGlobalRestrictionCacheTraits, _Global)
}}}
