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
#include "tests/test/plugins/ValidatorTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace validators {

#define TEST_CLASS BlockTypeValidatorTests

	DEFINE_COMMON_VALIDATOR_TESTS(BlockType, 123, Height(111))

	namespace {
		void AssertValidationResult(
				ValidationResult expectedResult,
				uint64_t importanceGrouping,
				model::EntityType blockType,
				Height blockHeight,
				Height forkHeight) {
			// Arrange:
			auto notification = model::BlockTypeNotification(blockType, blockHeight);
			auto pValidator = CreateBlockTypeValidator(importanceGrouping, forkHeight);

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification);

			// Assert:
			EXPECT_EQ(expectedResult, result)
					<< "importanceGrouping = " << importanceGrouping
					<< ", blockType = " << blockType
					<< ", blockHeight = " << blockHeight
					<< ", forkHeight = " << forkHeight;
		}

		void AssertValidationResultMultiple(
				model::EntityType successBlockType,
				uint64_t importanceGrouping,
				Height blockHeight,
				Height forkHeight = Height(0)) {
			// Arrange:
			auto blockTypes = std::initializer_list<model::EntityType>{
				model::Entity_Type_Block_Nemesis,
				model::Entity_Type_Block_Normal,
				model::Entity_Type_Block_Importance,
				model::Entity_Type_Voting_Key_Link
			};

			for (auto blockType : blockTypes) {
				auto expectedResult = successBlockType == blockType ? ValidationResult::Success : Failure_Core_Unexpected_Block_Type;

				// Act + Assert:
				AssertValidationResult(expectedResult, importanceGrouping, blockType, blockHeight, forkHeight);
			}
		}
	}

	// region validation

	TEST(TEST_CLASS, HeightZero_MustBeNormalBlock) {
		AssertValidationResultMultiple(model::Entity_Type_Block_Normal, 50, Height(0));

		AssertValidationResultMultiple(model::Entity_Type_Block_Normal, 123, Height(0));
	}

	TEST(TEST_CLASS, HeightOne_MustBeNemesisBlock) {
		AssertValidationResultMultiple(model::Entity_Type_Block_Nemesis, 50, Height(1));

		AssertValidationResultMultiple(model::Entity_Type_Block_Nemesis, 123, Height(1));
	}

	TEST(TEST_CLASS, HeightImportanceGrouping_MustBeImportanceBlock) {
		AssertValidationResultMultiple(model::Entity_Type_Block_Importance, 50, Height(150));
		AssertValidationResultMultiple(model::Entity_Type_Block_Importance, 50, Height(300));

		AssertValidationResultMultiple(model::Entity_Type_Block_Importance, 123, Height(123));
		AssertValidationResultMultiple(model::Entity_Type_Block_Importance, 123, Height(246));
	}

	TEST(TEST_CLASS, HeightNotImportanceGrouping_MustBeNormalBlock) {
		AssertValidationResultMultiple(model::Entity_Type_Block_Normal, 50, Height(25));
		AssertValidationResultMultiple(model::Entity_Type_Block_Normal, 50, Height(49));
		AssertValidationResultMultiple(model::Entity_Type_Block_Normal, 50, Height(51));
		AssertValidationResultMultiple(model::Entity_Type_Block_Normal, 50, Height(75));

		AssertValidationResultMultiple(model::Entity_Type_Block_Normal, 123, Height(100));
		AssertValidationResultMultiple(model::Entity_Type_Block_Normal, 123, Height(122));
		AssertValidationResultMultiple(model::Entity_Type_Block_Normal, 123, Height(124));
		AssertValidationResultMultiple(model::Entity_Type_Block_Normal, 123, Height(200));
	}

	// endregion

	// region fork inflection

	TEST(TEST_CLASS, CorrectInflectionAtForkHeight_Importance) {
		AssertValidationResultMultiple(model::Entity_Type_Block_Normal, 123, Height(246), Height(247));
		AssertValidationResultMultiple(model::Entity_Type_Block_Normal, 123, Height(246), Height(246));
		AssertValidationResultMultiple(model::Entity_Type_Block_Importance, 123, Height(246), Height(245));
	}

	TEST(TEST_CLASS, CorrectInflectionAtForkHeight_Normal) {
		AssertValidationResultMultiple(model::Entity_Type_Block_Normal, 123, Height(200), Height(201));
		AssertValidationResultMultiple(model::Entity_Type_Block_Normal, 123, Height(200), Height(200));
		AssertValidationResultMultiple(model::Entity_Type_Block_Normal, 123, Height(200), Height(199));
	}

	// endregion
}}
