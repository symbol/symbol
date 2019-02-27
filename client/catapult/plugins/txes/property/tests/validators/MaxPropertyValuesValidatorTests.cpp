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
#include "sdk/src/extensions/ConversionExtensions.h"
#include "tests/test/PropertyCacheTestUtils.h"
#include "tests/test/core/AddressTestUtils.h"
#include "tests/test/plugins/ValidatorTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace validators {

#define TEST_CLASS MaxPropertyValuesValidatorTests

	DEFINE_COMMON_VALIDATOR_TESTS(MaxAddressPropertyValues, 5)
	DEFINE_COMMON_VALIDATOR_TESTS(MaxMosaicPropertyValues, 5)
	DEFINE_COMMON_VALIDATOR_TESTS(MaxTransactionTypePropertyValues, 5)

	namespace {
		constexpr auto Add = model::PropertyModificationType::Add;
		constexpr auto Del = model::PropertyModificationType::Del;

		struct AddressPropertyTraits : public test::BaseAddressPropertyTraits {
			static constexpr auto CreateValidator = CreateMaxAddressPropertyValuesValidator;

			using NotificationType = model::ModifyAddressPropertyNotification;

			static auto ToUnresolved(const ValueType& value) {
				return extensions::CopyToUnresolvedAddress(value);
			}
		};

		struct MosaicPropertyTraits : public test::BaseMosaicPropertyTraits {
			static constexpr auto CreateValidator = CreateMaxMosaicPropertyValuesValidator;

			using NotificationType = model::ModifyMosaicPropertyNotification;

			static auto ToUnresolved(const ValueType& value) {
				return extensions::CastToUnresolvedMosaicId(value);
			}
		};

		struct TransactionTypePropertyTraits : public test::BaseTransactionTypePropertyTraits {
			static constexpr auto CreateValidator = CreateMaxTransactionTypePropertyValuesValidator;

			using NotificationType = model::ModifyTransactionTypePropertyNotification;

			static auto ToUnresolved(const ValueType& value) {
				return value;
			}
		};

		template<typename TPropertyValueTraits>
		void AssertValidationResult(
				ValidationResult expectedResult,
				uint16_t maxPropertyValues,
				uint16_t numInitialValues,
				uint16_t numAddModifications,
				uint16_t numDelModifications) {
			// Sanity:
			ASSERT_GE(numInitialValues, numDelModifications);

			// Arrange:
			auto initialValues = test::GenerateUniqueRandomDataVector<typename TPropertyValueTraits::ValueType>(numInitialValues);
			auto cache = test::PropertyCacheFactory::Create();
			auto key = test::GenerateRandomData<Key_Size>();
			test::PopulateCache<TPropertyValueTraits>(cache, key, initialValues);
			std::vector<model::PropertyModification<typename TPropertyValueTraits::UnresolvedValueType>> modifications;
			for (auto i = 0u; i < std::max(numAddModifications, numDelModifications); ++i) {
				if (i < numAddModifications)
					modifications.push_back({ Add, TPropertyValueTraits::RandomUnresolvedValue() });

				if (i < numDelModifications)
					modifications.push_back({ Del, TPropertyValueTraits::ToUnresolved(initialValues[i]) });
			}

			auto pValidator = TPropertyValueTraits::CreateValidator(maxPropertyValues);

			using UnresolvedValueType = typename TPropertyValueTraits::UnresolvedValueType;
			auto notification = test::CreateNotification<TPropertyValueTraits, UnresolvedValueType>(key, modifications);

			// Act:
			auto result = test::ValidateNotification<typename TPropertyValueTraits::NotificationType>(*pValidator, notification, cache);

			// Assert:
			EXPECT_EQ(expectedResult, result);
		}
	}

#define TRAITS_BASED_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_Address) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<AddressPropertyTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Mosaic) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<MosaicPropertyTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_TransactionType) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<TransactionTypePropertyTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	TRAITS_BASED_TEST(FailureWhenMaxModificationsIsExceeded) {
		// Assert:
		AssertValidationResult<TTraits>(Failure_Property_Modification_Count_Exceeded, 10, 0, 11, 0);
	}

	TRAITS_BASED_TEST(FailureWhenResultingPropertyValuesExceedsMaximum) {
		// Assert:
		AssertValidationResult<TTraits>(Failure_Property_Values_Count_Exceeded, 10, 5, 6, 0);
		AssertValidationResult<TTraits>(Failure_Property_Values_Count_Exceeded, 10, 5, 8, 2);
	}

	TRAITS_BASED_TEST(SuccessWhenResultingPropertyValuesDoesNotExceedMaximum) {
		// Assert:
		AssertValidationResult<TTraits>(ValidationResult::Success, 10, 0, 9, 0);
		AssertValidationResult<TTraits>(ValidationResult::Success, 10, 0, 10, 0);
		AssertValidationResult<TTraits>(ValidationResult::Success, 10, 5, 5, 0);
		AssertValidationResult<TTraits>(ValidationResult::Success, 10, 5, 7, 2);
	}
}}
