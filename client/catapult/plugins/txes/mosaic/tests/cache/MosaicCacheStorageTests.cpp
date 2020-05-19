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

#include "src/cache/MosaicCacheStorage.h"
#include "src/cache/MosaicCache.h"
#include "tests/test/MosaicTestUtils.h"
#include "tests/test/cache/CacheStorageTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace cache {

	namespace {
		struct MosaicCacheStorageTraits {
			using StorageType = MosaicCacheStorage;
			class CacheType : public MosaicCache {
			public:
				CacheType() : MosaicCache(CacheConfiguration())
				{}
			};

			static auto CreateId(uint8_t id) {
				return MosaicId(id);
			}

			static auto CreateValue(MosaicId id) {
				auto properties = test::CreateMosaicPropertiesWithDuration(BlockDuration(37));
				auto definition = state::MosaicDefinition(Height(11), test::CreateRandomOwner(), 3, properties);
				return state::MosaicEntry(id, definition);
			}

			static void AssertEqual(const state::MosaicEntry& lhs, const state::MosaicEntry& rhs) {
				test::AssertEqual(lhs, rhs);
			}
		};
	}

	DEFINE_BASIC_INSERT_REMOVE_CACHE_STORAGE_TESTS(MosaicCacheStorageTests, MosaicCacheStorageTraits)
}}
