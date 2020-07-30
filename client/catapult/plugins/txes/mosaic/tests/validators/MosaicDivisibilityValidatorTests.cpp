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

#include "src/validators/Validators.h"
#include "tests/test/MosaicCacheTestUtils.h"
#include "tests/test/MosaicTestUtils.h"
#include "tests/test/plugins/ValidatorTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace validators {

#define TEST_CLASS MosaicDivisibilityValidatorTests

	DEFINE_COMMON_VALIDATOR_TESTS(MosaicDivisibility, 0)

	namespace {
		void AddMosaic(cache::CatapultCache& cache, const Address& owner, MosaicId mosaicId, uint8_t divisibility) {
			auto properties = model::MosaicProperties(model::MosaicFlags::None, divisibility, BlockDuration());
			auto definition = state::MosaicDefinition(Height(50), owner, 3, properties);
			auto mosaicEntry = state::MosaicEntry(mosaicId, definition);

			auto delta = cache.createDelta();
			delta.sub<cache::MosaicCache>().insert(mosaicEntry);
			cache.commit(Height());
		}

		void AssertDivisibilityValidationResult(
				ValidationResult expectedResult,
				uint8_t initialDivisibility,
				uint8_t notificationDivisibility,
				uint8_t maxDivisibility) {
			// Arrange:
			auto pValidator = CreateMosaicDivisibilityValidator(maxDivisibility);

			auto owner = test::CreateRandomOwner();
			auto mosaicId = MosaicId(123);
			auto properties = model::MosaicProperties(model::MosaicFlags::None, notificationDivisibility, BlockDuration());
			auto notification = model::MosaicDefinitionNotification(owner, mosaicId, properties);

			auto cache = test::MosaicCacheFactory::Create(model::BlockChainConfiguration::Uninitialized());
			if (0 < initialDivisibility)
				AddMosaic(cache, owner, mosaicId, initialDivisibility);

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification, cache);

			// Assert:
			EXPECT_EQ(expectedResult, result)
					<< "initial " << static_cast<uint32_t>(initialDivisibility)
					<< ", notification " << static_cast<uint32_t>(notificationDivisibility)
					<< ", max " << static_cast<uint32_t>(maxDivisibility);
		}

		void AssertDivisibilityValidationResultRange(const consumer<uint8_t>& assertStep) {
			for (auto divisibility : std::initializer_list<uint8_t>{ 0, 5, 9 })
				assertStep(divisibility);
		}
	}

	TEST(TEST_CLASS, SuccessWhenValidatingDivisibilityLessThanMax_New) {
		AssertDivisibilityValidationResultRange([](auto divisibility) {
			AssertDivisibilityValidationResult(ValidationResult::Success, 0, divisibility, 10);
		});
	}

	TEST(TEST_CLASS, SuccessWhenValidatingDivisibilityLessThanMax_Existing) {
		AssertDivisibilityValidationResultRange([](auto divisibility) {
			AssertDivisibilityValidationResult(ValidationResult::Success, 3, 3 ^ divisibility, 10);
		});

		// following is valid even though notification divisibility is greater than max divisibility because 10 ^ 11 == 1 < 10
		AssertDivisibilityValidationResult(ValidationResult::Success, 10, 11, 10);
	}

	TEST(TEST_CLASS, SuccessWhenValidatingDivisibilityEqualToMax_New) {
		AssertDivisibilityValidationResult(ValidationResult::Success, 0, 10, 10);
	}

	TEST(TEST_CLASS, SuccessWhenValidatingDivisibilityEqualToMax_Existing) {
		AssertDivisibilityValidationResultRange([](auto divisibility) {
			AssertDivisibilityValidationResult(ValidationResult::Success, static_cast<uint8_t>(10 ^ divisibility), divisibility, 10);
		});
	}

	TEST(TEST_CLASS, FailureWhenValidatingDivisibilityGreaterThanMax_New) {
		AssertDivisibilityValidationResultRange([](auto divisibility) {
			AssertDivisibilityValidationResult(Failure_Mosaic_Invalid_Divisibility, 0, static_cast<uint8_t>(11 + divisibility), 10);
		});
	}

	TEST(TEST_CLASS, FailureWhenValidatingDivisibilityGreaterThanMax_Existing) {
		AssertDivisibilityValidationResultRange([](auto divisibility) {
			AssertDivisibilityValidationResult(Failure_Mosaic_Invalid_Divisibility, 3, static_cast<uint8_t>(3 ^ (11 + divisibility)), 10);
		});
	}
}}
