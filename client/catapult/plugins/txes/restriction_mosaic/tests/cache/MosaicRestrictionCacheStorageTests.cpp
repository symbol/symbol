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

#include "src/cache/MosaicRestrictionCacheStorage.h"
#include "src/cache/MosaicRestrictionCache.h"
#include "src/model/MosaicRestrictionTypes.h"
#include "tests/test/MosaicRestrictionTestUtils.h"
#include "tests/test/cache/CacheStorageTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace cache {

	namespace {
		struct MosaicRestrictionCacheStorageTraits {
			using StorageType = MosaicRestrictionCacheStorage;
			class CacheType : public MosaicRestrictionCache {
			public:
				CacheType() : MosaicRestrictionCache(CacheConfiguration(), static_cast<model::NetworkIdentifier>(12))
				{}
			};

			static auto CreateId(uint8_t id) {
				return state::CreateMosaicRestrictionEntryKey(MosaicId(id * id), Address{ { id } });
			}

			static auto CreateValue(const Hash256& hash) {
				state::MosaicRestrictionEntry entry(test::GenerateMosaicRestrictionEntry(hash));
				auto& restriction = entry.asAddressRestriction();
				for (auto i = 0u; i < 3; ++i)
					restriction.set(i, i * i);

				return entry;
			}

			static void AssertEqual(const state::MosaicRestrictionEntry& lhs, const state::MosaicRestrictionEntry& rhs) {
				test::AssertEqual(lhs, rhs);
			}
		};
	}

	DEFINE_BASIC_INSERT_REMOVE_CACHE_STORAGE_TESTS(MosaicRestrictionCacheStorageTests, MosaicRestrictionCacheStorageTraits)
}}
