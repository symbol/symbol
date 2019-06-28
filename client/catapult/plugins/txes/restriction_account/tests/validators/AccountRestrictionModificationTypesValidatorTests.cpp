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
#include "tests/test/AccountRestrictionCacheTestUtils.h"
#include "tests/test/core/AddressTestUtils.h"
#include "tests/test/plugins/ValidatorTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace validators {

#define TEST_CLASS AccountRestrictionModificationTypesValidatorTests

	DEFINE_COMMON_VALIDATOR_TESTS(AccountAddressRestrictionModificationTypes,)
	DEFINE_COMMON_VALIDATOR_TESTS(AccountMosaicRestrictionModificationTypes,)
	DEFINE_COMMON_VALIDATOR_TESTS(AccountOperationRestrictionModificationTypes,)

	namespace {
		constexpr auto Add = model::AccountRestrictionModificationType::Add;
		constexpr auto Del = model::AccountRestrictionModificationType::Del;

		struct AccountAddressRestrictionTraits : public test::BaseAccountAddressRestrictionTraits {
			static constexpr auto CreateValidator = CreateAccountAddressRestrictionModificationTypesValidator;

			using NotificationType = model::ModifyAccountAddressRestrictionNotification;
		};

		struct AccountMosaicRestrictionTraits : public test::BaseAccountMosaicRestrictionTraits {
			static constexpr auto CreateValidator = CreateAccountMosaicRestrictionModificationTypesValidator;

			using NotificationType = model::ModifyAccountMosaicRestrictionNotification;
		};

		struct AccountOperationRestrictionTraits : public test::BaseAccountOperationRestrictionTraits {
			static constexpr auto CreateValidator = CreateAccountOperationRestrictionModificationTypesValidator;

			using NotificationType = model::ModifyAccountOperationRestrictionNotification;
		};

		template<typename TRestrictionValueTraits>
		std::vector<model::AccountRestrictionModification<typename TRestrictionValueTraits::UnresolvedValueType>> CreateModifications(
				const std::vector<model::AccountRestrictionModificationType>& modificationTypes) {
			std::vector<model::AccountRestrictionModification<typename TRestrictionValueTraits::UnresolvedValueType>> modifications;
			for (auto modificationType : modificationTypes)
				modifications.push_back({ modificationType, TRestrictionValueTraits::RandomUnresolvedValue() });

			return modifications;
		}

		template<typename TRestrictionValueTraits>
		void AssertValidationResult(
				ValidationResult expectedResult,
				const std::vector<model::AccountRestrictionModificationType>& modificationTypes) {
			// Arrange:
			auto modifications = CreateModifications<TRestrictionValueTraits>(modificationTypes);
			typename TRestrictionValueTraits::NotificationType notification(
					test::GenerateRandomByteArray<Key>(),
					TRestrictionValueTraits::Restriction_Type,
					static_cast<uint8_t>(modifications.size()),
					modifications.data());
			auto pValidator = TRestrictionValueTraits::CreateValidator();

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification);

			// Assert:
			EXPECT_EQ(expectedResult, result);
		}

		model::AccountRestrictionModificationType CreateType(uint8_t value) {
			return static_cast<model::AccountRestrictionModificationType>(value);
		}
	}

#define TRAITS_BASED_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_Address) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<AccountAddressRestrictionTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Mosaic) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<AccountMosaicRestrictionTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Operation) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<AccountOperationRestrictionTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	TRAITS_BASED_TEST(FailureWhenValidatingNotificationWithUnknownAccountRestrictionModificationType) {
		// Assert:
		AssertValidationResult<TTraits>(Failure_RestrictionAccount_Modification_Type_Invalid, { Add, Del, CreateType(0x02), Del });
		AssertValidationResult<TTraits>(Failure_RestrictionAccount_Modification_Type_Invalid, { CreateType(0x12) });
		AssertValidationResult<TTraits>(Failure_RestrictionAccount_Modification_Type_Invalid, { CreateType(0xFF), CreateType(0x12) });
	}

	TRAITS_BASED_TEST(SuccessWhenValidatingNotificationWithNoAccountRestrictionModificationTypes) {
		// Assert:
		AssertValidationResult<TTraits>(ValidationResult::Success, {});
	}

	TRAITS_BASED_TEST(SuccessWhenValidatingNotificationWithKnownAccountRestrictionModificationTypes) {
		// Assert:
		AssertValidationResult<TTraits>(ValidationResult::Success, { Add, Add, Del, Add, Del, Del });
	}
}}
