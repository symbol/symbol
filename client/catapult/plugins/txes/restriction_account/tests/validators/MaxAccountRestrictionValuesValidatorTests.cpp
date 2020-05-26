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
#include "tests/test/AccountRestrictionCacheTestUtils.h"
#include "tests/test/core/AddressTestUtils.h"
#include "tests/test/plugins/ValidatorTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace validators {

#define TEST_CLASS MaxAccountRestrictionValuesValidatorTests

	DEFINE_COMMON_VALIDATOR_TESTS(MaxAccountAddressRestrictionValues, 5)
	DEFINE_COMMON_VALIDATOR_TESTS(MaxAccountMosaicRestrictionValues, 5)
	DEFINE_COMMON_VALIDATOR_TESTS(MaxAccountOperationRestrictionValues, 5)

	namespace {
		struct AccountAddressRestrictionTraits : public test::BaseAccountAddressRestrictionTraits {
			static constexpr auto CreateValidator = CreateMaxAccountAddressRestrictionValuesValidator;

			using NotificationType = model::ModifyAccountAddressRestrictionsNotification;

			static auto ToUnresolved(const ValueType& value) {
				return extensions::CopyToUnresolvedAddress(value);
			}
		};

		struct AccountMosaicRestrictionTraits : public test::BaseAccountMosaicRestrictionTraits {
			static constexpr auto CreateValidator = CreateMaxAccountMosaicRestrictionValuesValidator;

			using NotificationType = model::ModifyAccountMosaicRestrictionsNotification;

			static auto ToUnresolved(const ValueType& value) {
				return extensions::CastToUnresolvedMosaicId(value);
			}
		};

		struct AccountOperationRestrictionTraits : public test::BaseAccountOperationRestrictionTraits {
			static constexpr auto CreateValidator = CreateMaxAccountOperationRestrictionValuesValidator;

			using NotificationType = model::ModifyAccountOperationRestrictionsNotification;

			static auto ToUnresolved(const ValueType& value) {
				return value;
			}
		};

		template<typename TRestrictionValueTraits>
		void AssertValidationResult(
				ValidationResult expectedResult,
				uint16_t maxAccountRestrictionValues,
				uint16_t numInitialValues,
				uint8_t numAdditions,
				uint8_t numDeletions) {
			// Sanity:
			ASSERT_GE(numInitialValues, numDeletions);

			// Arrange:
			auto initialValues = test::GenerateUniqueRandomDataVector<typename TRestrictionValueTraits::ValueType>(numInitialValues);
			auto cache = test::AccountRestrictionCacheFactory::Create();
			auto address = test::GenerateRandomByteArray<Address>();
			test::PopulateCache<TRestrictionValueTraits>(cache, address, initialValues);

			std::vector<typename TRestrictionValueTraits::UnresolvedValueType> restrictionAdditions;
			for (auto i = 0u; i < numAdditions; ++i)
				restrictionAdditions.push_back(TRestrictionValueTraits::RandomUnresolvedValue());

			std::vector<typename TRestrictionValueTraits::UnresolvedValueType> restrictionDeletions;
			for (auto i = 0u; i < numDeletions; ++i)
				restrictionDeletions.push_back(TRestrictionValueTraits::ToUnresolved(initialValues[i]));

			auto pValidator = TRestrictionValueTraits::CreateValidator(maxAccountRestrictionValues);
			auto notification = test::CreateAccountRestrictionsNotification<TRestrictionValueTraits>(
					address,
					restrictionAdditions,
					restrictionDeletions);

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification, cache);

			// Assert:
			EXPECT_EQ(expectedResult, result) << "numAdditions " << numAdditions << ", numDeletions " << numDeletions;
		}
	}

#define TRAITS_BASED_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_Address) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<AccountAddressRestrictionTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Mosaic) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<AccountMosaicRestrictionTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Operation) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<AccountOperationRestrictionTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	TRAITS_BASED_TEST(FailureWhenMaxModificationsIsExceeded) {
		AssertValidationResult<TTraits>(Failure_RestrictionAccount_Modification_Count_Exceeded, 10, 0, 11, 0);
		AssertValidationResult<TTraits>(Failure_RestrictionAccount_Modification_Count_Exceeded, 10, 5, 6, 5);
		AssertValidationResult<TTraits>(Failure_RestrictionAccount_Modification_Count_Exceeded, 10, 6, 5, 6);
		AssertValidationResult<TTraits>(Failure_RestrictionAccount_Modification_Count_Exceeded, 10, 11, 0, 11);
	}

	TRAITS_BASED_TEST(FailureWhenResultingAccountRestrictionValuesExceedsMaximum) {
		AssertValidationResult<TTraits>(Failure_RestrictionAccount_Values_Count_Exceeded, 10, 5, 6, 0);
		AssertValidationResult<TTraits>(Failure_RestrictionAccount_Values_Count_Exceeded, 10, 5, 8, 2);
	}

	TRAITS_BASED_TEST(SuccessWhenResultingAccountRestrictionValuesDoesNotExceedMaximum) {
		AssertValidationResult<TTraits>(ValidationResult::Success, 10, 0, 9, 0);
		AssertValidationResult<TTraits>(ValidationResult::Success, 10, 0, 10, 0);
		AssertValidationResult<TTraits>(ValidationResult::Success, 10, 5, 5, 0);
		AssertValidationResult<TTraits>(ValidationResult::Success, 10, 5, 7, 2);
	}
}}
