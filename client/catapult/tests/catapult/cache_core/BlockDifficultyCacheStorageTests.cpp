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

#define TEST_CLASS BlockDifficultyCacheStorageTests

	namespace {
		struct BlockDifficultyCacheStorageTraits {
			using ValueType = state::BlockDifficultyInfo;
			static constexpr auto Value_Size = sizeof(Height) + sizeof(Timestamp) + sizeof(Difficulty);

			using StorageType = BlockDifficultyCacheStorage;
			class CacheType : public BlockDifficultyCache {
			public:
				CacheType() : BlockDifficultyCache(845)
				{}
			};

			static auto CreateValue(uint8_t id) {
				return ValueType(Height(id), test::GenerateRandomValue<Timestamp>(), test::GenerateRandomValue<Difficulty>());
			}
		};
	}

	DEFINE_CONTAINS_ONLY_CACHE_STORAGE_TESTS(TEST_CLASS, BlockDifficultyCacheStorageTraits)

	TEST(TEST_CLASS, CanPurgeValueWhenValuesWithLargerHeightsArePresent) {
		// Arrange: seed the cache with some values
		static constexpr auto CreateValue = BlockDifficultyCacheStorageTraits::CreateValue;
		BlockDifficultyCacheStorageTraits::CacheType cache;
		{
			auto delta = cache.createDelta();
			for (uint8_t i = 1; 10 >= i; ++i)
				delta->insert(CreateValue(i));

			cache.commit();
		}

		// Sanity:
		EXPECT_EQ(10u, cache.createView()->size());

		// Act:
		auto delta = cache.createDelta();
		BlockDifficultyCacheStorageTraits::StorageType::Purge(CreateValue(7), *delta);

		// Assert:
		EXPECT_EQ(6u, delta->size());

		for (uint8_t i = 1; 7 > i; ++i)
			EXPECT_TRUE(delta->contains(CreateValue(i))) << static_cast<uint16_t>(i);

		for (uint8_t i = 7; 10 >= i; ++i)
			EXPECT_FALSE(delta->contains(CreateValue(i))) << static_cast<uint16_t>(i);
	}
}}
