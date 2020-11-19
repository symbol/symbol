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

#include "src/cache/MetadataCacheStorage.h"
#include "src/cache/MetadataCache.h"
#include "tests/test/MetadataTestUtils.h"
#include "tests/test/cache/CacheStorageTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace cache {

	namespace {
		struct MetadataCacheStorageTraits {
			using StorageType = MetadataCacheStorage;
			class CacheType : public MetadataCache {
			public:
				CacheType() : MetadataCache(CacheConfiguration())
				{}
			};

			static auto CreateId(uint8_t id) {
				return test::CreateMetadataUniqueKeyFromSeed(id);
			}

			static auto CreateValue(const Hash256& hash) {
				state::MetadataEntry entry(test::GenerateMetadataKey(hash));

				std::vector<uint8_t> valueBuffer{ 0x9A, 0xC7, 0x33 };
				entry.value().update(valueBuffer);

				return entry;
			}

			static void AssertEqual(const state::MetadataEntry& lhs, const state::MetadataEntry& rhs) {
				test::AssertEqual(lhs, rhs);
			}
		};
	}

	DEFINE_BASIC_INSERT_REMOVE_CACHE_STORAGE_TESTS(MetadataCacheStorageTests, MetadataCacheStorageTraits)
}}
