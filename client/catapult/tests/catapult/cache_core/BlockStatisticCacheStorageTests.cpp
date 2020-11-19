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

#include "catapult/cache_core/BlockStatisticCacheStorage.h"
#include "tests/test/cache/CacheStorageTestUtils.h"
#include "tests/test/core/SerializerTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace cache {

#define TEST_CLASS BlockStatisticCacheStorageTests

	namespace {
		struct BlockStatisticCacheStorageTraits {
			using ValueType = state::BlockStatistic;
			static constexpr auto Value_Size = sizeof(Height) + sizeof(Timestamp) + sizeof(Difficulty) + sizeof(BlockFeeMultiplier);

			using StorageType = BlockStatisticCacheStorage;
			class CacheType : public BlockStatisticCache {
			public:
				CacheType() : BlockStatisticCache(845)
				{}
			};

			static auto CreateValue(uint8_t id) {
				// set all statistic fields to nonzero values
				return ValueType(Height(id), Timestamp(id + 1), Difficulty() + Difficulty::Unclamped(id + 2), BlockFeeMultiplier(id + 3));
			}
		};
	}

	DEFINE_CONTAINS_ONLY_CACHE_STORAGE_TESTS(TEST_CLASS, BlockStatisticCacheStorageTraits)

	TEST(TEST_CLASS, CanRoundtripBlockStatistic) {
		// Arrange:
		auto originalStatistic = BlockStatisticCacheStorageTraits::CreateValue(123);

		// Act:
		auto result = test::RunRoundtripBufferTest<BlockStatisticCacheStorage>(originalStatistic);

		// Assert:
		EXPECT_EQ(Height(123), result.Height);
		EXPECT_EQ(Timestamp(124), result.Timestamp);
		EXPECT_EQ(Difficulty() + Difficulty::Unclamped(125), result.Difficulty);
		EXPECT_EQ(BlockFeeMultiplier(126), result.FeeMultiplier);
	}

	TEST(TEST_CLASS, CanPurgeValueWhenValuesWithLargerHeightsArePresent) {
		// Arrange: seed the cache with some values
		static constexpr auto CreateValue = BlockStatisticCacheStorageTraits::CreateValue;
		BlockStatisticCacheStorageTraits::CacheType cache;
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
		BlockStatisticCacheStorageTraits::StorageType::Purge(CreateValue(7), *delta);

		// Assert:
		EXPECT_EQ(6u, delta->size());

		for (uint8_t i = 1; 7 > i; ++i)
			EXPECT_TRUE(delta->contains(CreateValue(i))) << static_cast<uint16_t>(i);

		for (uint8_t i = 7; 10 >= i; ++i)
			EXPECT_FALSE(delta->contains(CreateValue(i))) << static_cast<uint16_t>(i);
	}
}}
