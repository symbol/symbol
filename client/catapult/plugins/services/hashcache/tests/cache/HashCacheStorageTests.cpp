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

#include "src/cache/HashCacheStorage.h"
#include "tests/test/cache/CacheStorageTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace cache {

	namespace {
		struct HashCacheStorageTraits {
			using ValueType = state::TimestampedHash;
			static constexpr auto Value_Size = sizeof(Timestamp) + sizeof(ValueType::HashType);

			using StorageType = HashCacheStorage;
			class CacheType : public HashCache {
			public:
				CacheType() : HashCache(CacheConfiguration(), utils::TimeSpan::FromHours(32))
				{}
			};

			static auto CreateValue(uint8_t id) {
				return ValueType(Timestamp(id), Hash256{ { id } });
			}
		};
	}

	DEFINE_CONTAINS_ONLY_CACHE_STORAGE_TESTS(HashCacheStorageTests, HashCacheStorageTraits)
}}
