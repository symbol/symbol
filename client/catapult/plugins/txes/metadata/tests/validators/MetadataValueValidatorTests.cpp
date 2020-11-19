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

#include "src/validators/Validators.h"
#include "tests/test/MetadataCacheTestUtils.h"
#include "tests/test/MetadataTestUtils.h"
#include "tests/test/plugins/ValidatorTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace validators {

#define TEST_CLASS MetadataValueValidatorTests

	DEFINE_COMMON_VALIDATOR_TESTS(MetadataValue,)

	// region test utils

	namespace {
		struct ValueSizes {
			int16_t Delta;
			uint16_t New;
		};

		auto CreateNotification(const state::MetadataKey& metadataKey, const ValueSizes& valueSizes, const uint8_t* pValue) {
			return test::CreateMetadataValueNotification(metadataKey, valueSizes.Delta, valueSizes.New, pValue);
		}
	}

	// endregion

	// region entry not in cache

	namespace {
		void RunEntryNotInCacheTest(ValidationResult expectedResult, int16_t valueSizeDelta, uint16_t valueSize) {
			// Arrange:
			auto pValidator = CreateMetadataValueValidator();

			auto metadataKey = test::GenerateRandomMetadataKey();
			auto valueBuffer = test::GenerateRandomVector(10);
			auto notification = CreateNotification(metadataKey, { valueSizeDelta, valueSize }, valueBuffer.data());

			auto cache = test::MetadataCacheFactory::Create();

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification, cache);

			// Assert:
			EXPECT_EQ(expectedResult, result) << "valueSizeDelta " << valueSizeDelta;
		}
	}

	TEST(TEST_CLASS, FailureWhenEntryNotInCacheAndValueSizeDeltaIsNotEqualToValueSize) {
		for (auto valueSizeDelta : std::initializer_list<int16_t>{ -100, -10, -8, -1, 1, 8, 10, 100 })
			RunEntryNotInCacheTest(Failure_Metadata_Value_Size_Delta_Mismatch, valueSizeDelta, 9);
	}

	TEST(TEST_CLASS, SuccessWhenEntryNotInCacheAndValueSizeDeltaIsEqualToValueSize) {
		for (auto valueSizeDelta : std::initializer_list<int16_t>{ 1, 8, 10, 100 })
			RunEntryNotInCacheTest(ValidationResult::Success, valueSizeDelta, static_cast<uint16_t>(valueSizeDelta));
	}

	// endregion

	// region entry in cache

	namespace {
		struct EntryInCacheTestSeed {
			ValueSizes Sizes;
			std::vector<uint8_t> CacheValue;
			std::vector<uint8_t> NotificationValue;
		};

		void RunEntryInCacheTest(ValidationResult expectedResult, const EntryInCacheTestSeed& testSeed) {
			// Arrange:
			auto pValidator = CreateMetadataValueValidator();

			auto metadataKey = test::GenerateRandomMetadataKey();
			auto notification = CreateNotification(metadataKey, testSeed.Sizes, testSeed.NotificationValue.data());

			auto cache = test::MetadataCacheFactory::Create();
			{
				auto delta = cache.createDelta();
				state::MetadataEntry metadataEntry(metadataKey);
				metadataEntry.value().update(testSeed.CacheValue);
				delta.sub<cache::MetadataCache>().insert(metadataEntry);
				cache.commit(Height());
			}

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification, cache);

			// Assert:
			EXPECT_EQ(expectedResult, result);
		}
	}

	TEST(TEST_CLASS, FailureWhenCacheValueSizeAndExpectedValueSizeDoNotMatch) {
		// Assert: cache value size = 5; expected value size = 4
		RunEntryInCacheTest(Failure_Metadata_Value_Size_Delta_Mismatch, {
			{ 0, 4 },
			{ 0xBA, 0xE8, 0xBC, 0x4B, 0x4B },
			{ 0xBA, 0xE8, 0xBC, 0x4B }
		});
		// - cache value size = 5; expected value size = 6
		RunEntryInCacheTest(Failure_Metadata_Value_Size_Delta_Mismatch, {
			{ 1, 7 },
			{ 0xE0, 0x77, 0xCC, 0xCC, 0x02 },
			{ 0xBA, 0xE8, 0xBC, 0x4B, 0x4B, 0xEE, 0x1F }
		});
		// - cache value size = 5; expected value size = 7
		RunEntryInCacheTest(Failure_Metadata_Value_Size_Delta_Mismatch, {
			{ -2, 7 },
			{ 0xBA, 0xE8, 0xBC, 0x4B, 0x4B },
			{ 0xE0, 0x77, 0xCC, 0xCC, 0x4B, 0xEE, 0x1F }
		});
	}

	TEST(TEST_CLASS, SuccessWhenCacheValueSizeIsEqualToNewValueSize) {
		RunEntryInCacheTest(ValidationResult::Success, {
			{ 0, 5 },
			{ 0xE0, 0x77, 0xCC, 0xCC, 0x02 },
			{ 0xBA, 0xE8, 0xBC, 0x4B, 0x4B }
		});
	}

	TEST(TEST_CLASS, SuccessWhenCacheValueSizeIsLessThanNewValueSize) {
		RunEntryInCacheTest(ValidationResult::Success, {
			{ 2, 7 },
			{ 0xE0, 0x77, 0xCC, 0xCC, 0x02 },
			{ 0xBA, 0xE8, 0xBC, 0x4B, 0x4B, 0xEE, 0x1F }
		});
	}

	TEST(TEST_CLASS, SuccessWhenCacheValueSizeIsGreaterThanNewValueSizeAndTruncationIsReversible) {
		RunEntryInCacheTest(ValidationResult::Success, {
			{ -3, 7 },
			{ 0xBA, 0xE8, 0xBC, 0x4B, 0x4B, 0xEE, 0x1F },
			{ 0xE0, 0x77, 0xCC, 0xCC, 0x4B, 0xEE, 0x1F }
		});
	}

	TEST(TEST_CLASS, FailureWhenCacheValueSizeIsGreaterThanNewValueSizeButTruncationIsNotReversible) {
		RunEntryInCacheTest(Failure_Metadata_Value_Change_Irreversible, {
			{ -3, 7 },
			{ 0xBA, 0xE8, 0xBC, 0x4B, 0x4B, 0xEE, 0x1F },
			{ 0xE0, 0x77, 0xCC, 0xCC, 0x4B, 0xEF, 0x1F }
		});
	}

	// endregion
}}
