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

#include "src/storages/MongoMultisigCacheStorage.h"
#include "mongo/src/mappers/MapperUtils.h"
#include "catapult/model/Address.h"
#include "mongo/tests/test/MongoFlatCacheStorageTests.h"
#include "mongo/tests/test/MongoTestUtils.h"
#include "plugins/txes/multisig/tests/test/MultisigCacheTestUtils.h"
#include "tests/test/MultisigMapperTestUtils.h"
#include "tests/TestHarness.h"

using namespace bsoncxx::builder::stream;

namespace catapult { namespace mongo { namespace plugins {

#define TEST_CLASS MongoMultisigCacheStorageTests

	namespace {
		void InsertRandom(state::SortedAddressSet& addresses, size_t count) {
			for (auto i = 0u; i < count; ++i)
				addresses.insert(test::GenerateRandomByteArray<Address>());
		}

		struct MultisigCacheTraits {
			using CacheType = cache::MultisigCache;
			using ModelType = state::MultisigEntry;

			static constexpr auto Collection_Name = "multisigs";
			static constexpr auto Network_Id = static_cast<model::NetworkIdentifier>(0x5A);
			static constexpr auto CreateCacheStorage = CreateMongoMultisigCacheStorage;

			static cache::CatapultCache CreateCache() {
				return test::MultisigCacheFactory::Create();
			}

			static ModelType GenerateRandomElement(uint32_t id) {
				auto address = Address();
				std::memcpy(address.data(), &id, sizeof(id));

				state::MultisigEntry entry(address);
				entry.setMinApproval(12);
				entry.setMinRemoval(23);

				InsertRandom(entry.cosignatoryAddresses(), std::max<size_t>(1, test::RandomByte() & 0x0F));
				InsertRandom(entry.multisigAddresses(), std::max<size_t>(1, test::RandomByte() & 0x0F));
				return entry;
			}

			static void Add(cache::CatapultCacheDelta& delta, const ModelType& entry) {
				auto& multisigCacheDelta = delta.sub<cache::MultisigCache>();
				multisigCacheDelta.insert(entry);
			}

			static void Remove(cache::CatapultCacheDelta& delta, const ModelType& entry) {
				auto& multisigCacheDelta = delta.sub<cache::MultisigCache>();
				multisigCacheDelta.remove(entry.address());
			}

			static void Mutate(cache::CatapultCacheDelta& delta, ModelType& entry) {
				// update expected
				entry.setMinApproval(24);

				// update cache
				auto& multisigCacheDelta = delta.sub<cache::MultisigCache>();
				auto& entryFromCache = multisigCacheDelta.find(entry.address()).get();
				entryFromCache.setMinApproval(24);
			}

			static auto GetFindFilter(const ModelType& entry) {
				return document() << "multisig.accountAddress" << mappers::ToBinary(entry.address()) << finalize;
			}

			static void AssertEqual(const ModelType& entry, const bsoncxx::document::view& view) {
				test::AssertEqualMultisigData(entry, view["multisig"].get_document().view());
			}
		};
	}

	DEFINE_FLAT_CACHE_STORAGE_TESTS(MultisigCacheTraits,)
}}}
