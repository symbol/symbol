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
#include "tests/test/plugins/ValidatorTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace validators {

#define TEST_CLASS NamespaceTypeValidatorTests

	DEFINE_COMMON_VALIDATOR_TESTS(NamespaceType,)

	// region namespace type

	namespace {
		void AssertNamespaceTypeResult(ValidationResult expectedResult, model::NamespaceType namespaceType) {
			// Arrange:
			auto pValidator = CreateNamespaceTypeValidator();
			auto notification = model::NamespaceNotification(namespaceType);

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification);

			// Assert:
			EXPECT_EQ(expectedResult, result) << "namespaceType " << static_cast<uint16_t>(namespaceType);
		}
	}

	TEST(TEST_CLASS, SuccessWhenValidatingValidNamespaceType) {
		// Assert:
		for (auto namespaceType : { 0x00, 0x01 })
			AssertNamespaceTypeResult(ValidationResult::Success, static_cast<model::NamespaceType>(namespaceType));
	}

	TEST(TEST_CLASS, FailureWhenValidatingInvalidNamespaceType) {
		// Assert:
		for (auto namespaceType : { 0x02, 0xFF })
			AssertNamespaceTypeResult(Failure_Namespace_Invalid_Namespace_Type, static_cast<model::NamespaceType>(namespaceType));
	}

	// endregion
}}
