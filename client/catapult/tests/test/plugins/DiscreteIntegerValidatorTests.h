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

#pragma once
#include "ValidatorTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace test {

	/// Container of discrete integer validator tests.
	template<typename TTraits>
	struct DiscreteIntegerValidatorTests {
	private:
		using EnumType = typename TTraits::EnumType;

	public:
		static void AssertSuccessWhenProcessingValidValue() {
			// Assert:
			for (auto value : TTraits::ValidValues())
				AssertValueValidationResult(validators::ValidationResult::Success, static_cast<EnumType>(value));
		}

		static void AssertFailureWhenProcessingInvalidValue() {
			// Assert:
			for (auto value : TTraits::InvalidValues())
				AssertValueValidationResult(TTraits::Failure_Result, static_cast<EnumType>(value));
		}

	private:
		static void AssertValueValidationResult(validators::ValidationResult expectedResult, EnumType value) {
			// Arrange:
			auto pValidator = TTraits::CreateValidator();
			auto notification = TTraits::CreateNotification(value);

			// Act:
			auto result = ValidateNotification(*pValidator, notification);

			// Assert:
			EXPECT_EQ(expectedResult, result) << "value " << static_cast<uint64_t>(value);
		}
	};

#define MAKE_DISCRETE_INTEGER_VALIDATOR_TEST(TEST_CLASS, TRAITS_NAME, TEST_NAME) \
	TEST(TEST_CLASS, TEST_NAME) { test::DiscreteIntegerValidatorTests<TRAITS_NAME>::Assert##TEST_NAME(); }

#define DEFINE_DISCRETE_INTEGER_VALIDATOR_TESTS(TEST_CLASS, TRAITS_NAME) \
	MAKE_DISCRETE_INTEGER_VALIDATOR_TEST(TEST_CLASS, TRAITS_NAME, SuccessWhenProcessingValidValue) \
	MAKE_DISCRETE_INTEGER_VALIDATOR_TEST(TEST_CLASS, TRAITS_NAME, FailureWhenProcessingInvalidValue)
}}
