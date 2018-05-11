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

#include "catapult/cache_core/BlockDifficultyCacheStorage.h"
#include "tests/test/cache/CacheStorageTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace cache {

	namespace {
		struct BlockDifficultyCacheStorageTraits{
			using ValueType = state::BlockDifficultyInfo;
			static constexpr auto Value_Size = sizeof(Height) + sizeof(Timestamp) + sizeof(Difficulty);

			using StorageType = BlockDifficultyCacheStorage;
			class CacheType : public BlockDifficultyCache {
			public:
				CacheType() : BlockDifficultyCache(845)
				{}
			};

			static auto CreateRandomValue() {
				return ValueType(
						test::GenerateRandomValue<Height>(),
						test::GenerateRandomValue<Timestamp>(),
						test::GenerateRandomValue<Difficulty>());
			}
		};
	}

	DEFINE_CONTAINS_ONLY_CACHE_STORAGE_TESTS(BlockDifficultyCacheStorageTests, BlockDifficultyCacheStorageTraits)
}}
