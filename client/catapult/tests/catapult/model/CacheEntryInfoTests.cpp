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

#include "catapult/model/CacheEntryInfo.h"
#include "catapult/utils/MemoryUtils.h"
#include "tests/test/core/VariableSizedEntityTestUtils.h"
#include "tests/test/nodeps/NumericTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace model {

#define TEST_CLASS CacheEntryInfoTests

	namespace {
		constexpr auto Max_Data_Size = CacheEntryInfo<uint64_t>::Max_Data_Size;
	}

	TEST(TEST_CLASS, EntityHasExpectedSize) {
		// Arrange:
		auto expectedSize =
				sizeof(uint32_t) // Size
				+ sizeof(uint64_t) // Id
				+ sizeof(uint32_t); // DataSize

		// Assert:
		EXPECT_EQ(expectedSize, sizeof(CacheEntryInfo<uint64_t>));
		EXPECT_EQ(16u, sizeof(CacheEntryInfo<uint64_t>));
	}

	// region CalculateRealSize

	TEST(TEST_CLASS, CanCalculateRealSizeWithReasonableValues) {
		// Arrange:
		CacheEntryInfo<uint64_t> info;
		info.Size = 0;
		info.DataSize = 100;

		// Act:
		auto realSize = CacheEntryInfo<uint64_t>::CalculateRealSize(info);

		// Assert:
		EXPECT_EQ(sizeof(CacheEntryInfo<uint64_t>) + 100, realSize);
	}

	TEST(TEST_CLASS, CalculateRealSizeDoesNotOverflowWithMaxValues) {
		// Arrange:
		CacheEntryInfo<uint64_t> info;
		info.Size = 0;
		info.DataSize = Max_Data_Size;

		// Act:
		auto realSize = CacheEntryInfo<uint64_t>::CalculateRealSize(info);

		// Assert:
		EXPECT_EQ(sizeof(CacheEntryInfo<uint64_t>) + info.DataSize, realSize);
		EXPECT_GE(std::numeric_limits<uint64_t>::max(), realSize);
	}

	// endregion

	// region HasData

	TEST(TEST_CLASS, HasDataReturnsFalseWhenDataSizeIsZero) {
		// Arrange:
		CacheEntryInfo<uint64_t> info;
		info.Size = 0;
		info.DataSize = 0;

		// Act:
		auto hasData = info.HasData();

		// Assert:
		EXPECT_FALSE(hasData);
	}

	TEST(TEST_CLASS, HasDataReturnsTrueWhenDataSizeIsGreaterThanZero) {
		// Arrange:
		CacheEntryInfo<uint64_t> info;
		info.Size = 0;
		info.DataSize = 10;

		// Act:
		auto hasData = info.HasData();

		// Assert:
		EXPECT_TRUE(hasData);
	}

	// endregion

	// region IsTooLarge

	namespace {
		bool IsTooLarge(uint32_t dataSize) {
			// Arrange:
			CacheEntryInfo<uint64_t> info;
			info.Size = 0;
			info.DataSize = dataSize;

			// Act:
			return info.IsTooLarge();
		}
	}

	TEST(TEST_CLASS, IsTooLargeReturnsFalseWhenDataSizeIsLessThanMax) {
		for (auto dataSize : { 0u, 100u, 10000u, Max_Data_Size - 1u })
			EXPECT_FALSE(IsTooLarge(dataSize)) << "for data size " << dataSize;
	}

	TEST(TEST_CLASS, IsTooLargeReturnsTrueWhenDataSizeIsGreaterOrEqualToMax) {
		for (auto dataSize : { Max_Data_Size, Max_Data_Size + 1, Max_Data_Size + 1000 })
			EXPECT_TRUE(IsTooLarge(dataSize)) << "for data size " << dataSize;
	}

	// endregion

	namespace {
		struct CacheEntryInfoTraits {
			static auto GenerateEntityWithAttachments(uint16_t count) {
				uint32_t entitySize = sizeof(CacheEntryInfo<uint64_t>) + count;
				auto pInfo = utils::MakeUniqueWithSize<CacheEntryInfo<uint64_t>>(entitySize);
				pInfo->Size = entitySize;
				pInfo->DataSize = count;
				return pInfo;
			}

			template<typename TEntity>
			static auto GetAttachmentPointer(TEntity& entity) {
				return entity.DataPtr();
			}
		};
	}

	DEFINE_ATTACHMENT_POINTER_TESTS(TEST_CLASS, CacheEntryInfoTraits) // DataPtr
}}
