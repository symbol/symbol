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

#include "src/storages/MongoMosaicCacheStorage.h"
#include "mongo/src/mappers/MapperUtils.h"
#include "mongo/tests/test/MongoFlatCacheStorageTests.h"
#include "mongo/tests/test/MongoTestUtils.h"
#include "plugins/txes/mosaic/tests/test/MosaicCacheTestUtils.h"
#include "plugins/txes/mosaic/tests/test/MosaicTestUtils.h"
#include "tests/test/MosaicMapperTestUtils.h"
#include "tests/TestHarness.h"

using namespace bsoncxx::builder::stream;

namespace catapult { namespace mongo { namespace plugins {

#define TEST_CLASS MongoMosaicCacheStorageTests

	namespace {
		struct MosaicCacheTraits {
			using CacheType = cache::MosaicCache;
			using ModelType = state::MosaicEntry;

			static constexpr auto Collection_Name = "mosaics";
			static constexpr auto Network_Id = static_cast<model::NetworkIdentifier>(0x5A);
			static constexpr auto CreateCacheStorage = CreateMongoMosaicCacheStorage;

			static cache::CatapultCache CreateCache() {
				return test::MosaicCacheFactory::Create();
			}

			static ModelType GenerateRandomElement(uint32_t id) {
				auto owner = test::CreateRandomOwner();
				return test::CreateMosaicEntry(MosaicId(id), Height(345), owner, Amount(456), BlockDuration(567));
			}

			static void Add(cache::CatapultCacheDelta& delta, const ModelType& mosaicEntry) {
				auto& mosaicCacheDelta = delta.sub<cache::MosaicCache>();
				mosaicCacheDelta.insert(mosaicEntry);
			}

			static void Remove(cache::CatapultCacheDelta& delta, const ModelType& mosaicEntry) {
				auto& mosaicCacheDelta = delta.sub<cache::MosaicCache>();
				mosaicCacheDelta.remove(mosaicEntry.mosaicId());
			}

			static void Mutate(cache::CatapultCacheDelta& delta, ModelType& mosaicEntry) {
				// update expected
				mosaicEntry.increaseSupply(Amount(1));

				// update cache
				auto& mosaicCacheDelta = delta.sub<cache::MosaicCache>();
				auto& mosaicEntryFromCache = mosaicCacheDelta.find(mosaicEntry.mosaicId()).get();
				mosaicEntryFromCache.increaseSupply(Amount(1));
			}

			static auto GetFindFilter(const ModelType& mosaicEntry) {
				return document() << "mosaic.id" << mappers::ToInt64(mosaicEntry.mosaicId()) << finalize;
			}

			static void AssertEqual(const ModelType& mosaicEntry, const bsoncxx::document::view& view) {
				test::AssertEqualMosaicData(mosaicEntry, view["mosaic"].get_document().view());
			}
		};
	}

	DEFINE_FLAT_CACHE_STORAGE_TESTS(MosaicCacheTraits,)
}}}
