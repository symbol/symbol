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
#include "tests/test/cache/CacheTestUtils.h"
#include "tests/test/plugins/ValidatorTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace validators {

#define TEST_CLASS MosaicFlagsValidatorTests

	DEFINE_COMMON_VALIDATOR_TESTS(MosaicFlags, Height())

	namespace {
		constexpr auto Revokable_Fork_Height = Height(1000);

		struct MosaicFlagsTraits {
			using EnumType = model::MosaicFlags;

			static constexpr auto Failure_Result = Failure_Mosaic_Invalid_Flags;
			static constexpr auto CreateValidator = CreateMosaicFlagsValidator;

			static std::vector<uint8_t> ValidValues() {
				return { 0x00, 0x02, 0x05, 0x07 }; // valid values prior to any fork
			}

			static std::vector<uint8_t> ValidValuesAfterRevokableFork() {
				return { 0x08, 0x09, 0x0F };
			}

			static std::vector<uint8_t> InvalidValuesAfterRevokableFork() {
				return { 0x10, 0x11, 0xFF };
			}

			static auto CreateNotification(model::MosaicFlags value) {
				model::MosaicProperties properties(value, 0, BlockDuration());
				return model::MosaicPropertiesNotification(properties);
			}
		};

		void AssertValueValidationResult(ValidationResult expectedResult, model::MosaicFlags value, Height height) {
			// Arrange:
			auto pValidator = MosaicFlagsTraits::CreateValidator(Revokable_Fork_Height);
			auto notification = MosaicFlagsTraits::CreateNotification(value);

			auto cache = test::CreateEmptyCatapultCache();
			auto cacheView = cache.createView();
			auto readOnlyCache = cacheView.toReadOnly();
			auto validatorContext = test::CreateValidatorContext(height, readOnlyCache);

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification, validatorContext);

			// Assert:
			EXPECT_EQ(expectedResult, result) << "value " << static_cast<uint64_t>(value) << " at " << height;
		}
	}

	// region basic

	TEST(TEST_CLASS, SuccessWhenProcessingValidValue) {
		for (auto value : MosaicFlagsTraits::ValidValues())
			AssertValueValidationResult(ValidationResult::Success, static_cast<model::MosaicFlags>(value), Height(1));
	}

	TEST(TEST_CLASS, FailureWhenProcessingInvalidValue) {
		for (auto value : MosaicFlagsTraits::ValidValuesAfterRevokableFork())
			AssertValueValidationResult(MosaicFlagsTraits::Failure_Result, static_cast<model::MosaicFlags>(value), Height(1));

		for (auto value : MosaicFlagsTraits::InvalidValuesAfterRevokableFork())
			AssertValueValidationResult(MosaicFlagsTraits::Failure_Result, static_cast<model::MosaicFlags>(value), Height(1));
	}

	// endregion

	// region revokable fork

	TEST(TEST_CLASS, FailureWhenRevokableFlagIsPresentBeforeRevokableFork) {
		for (auto value : MosaicFlagsTraits::ValidValuesAfterRevokableFork()) {
			for (auto heightAdjustment : { Height(1), Height(100) }) {
				auto height = Revokable_Fork_Height - heightAdjustment;
				AssertValueValidationResult(MosaicFlagsTraits::Failure_Result, static_cast<model::MosaicFlags>(value), height);
			}
		}
	}

	TEST(TEST_CLASS, SuccessWhenRevokableFlagIsPresentAtRevokableFork) {
		for (auto value : MosaicFlagsTraits::ValidValuesAfterRevokableFork())
			AssertValueValidationResult(ValidationResult::Success, static_cast<model::MosaicFlags>(value), Revokable_Fork_Height);
	}

	TEST(TEST_CLASS, SuccessWhenRevokableFlagIsPresentAfterRevokableFork) {
		for (auto value : MosaicFlagsTraits::ValidValuesAfterRevokableFork()) {
			for (auto heightAdjustment : { Height(1), Height(100) }) {
				auto height = Revokable_Fork_Height + heightAdjustment;
				AssertValueValidationResult(ValidationResult::Success, static_cast<model::MosaicFlags>(value), height);
			}
		}
	}

	TEST(TEST_CLASS, FailureWhenProcessingInvalidValueAtRevokableFork) {
		for (auto value : MosaicFlagsTraits::InvalidValuesAfterRevokableFork())
			AssertValueValidationResult(MosaicFlagsTraits::Failure_Result, static_cast<model::MosaicFlags>(value), Revokable_Fork_Height);
	}

	// endregion
}}
