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

#define TEST_CLASS AccountRestrictionModificationActionsValidatorTests

	DEFINE_COMMON_VALIDATOR_TESTS(AccountAddressRestrictionModificationActions,)
	DEFINE_COMMON_VALIDATOR_TESTS(AccountMosaicRestrictionModificationActions,)
	DEFINE_COMMON_VALIDATOR_TESTS(AccountOperationRestrictionModificationActions,)

	namespace {
		constexpr auto Add = model::AccountRestrictionModificationAction::Add;
		constexpr auto Del = model::AccountRestrictionModificationAction::Del;

		struct AccountAddressRestrictionTraits : public test::BaseAccountAddressRestrictionTraits {
			static constexpr auto CreateValidator = CreateAccountAddressRestrictionModificationActionsValidator;

			using NotificationType = model::ModifyAccountAddressRestrictionNotification;
		};

		struct AccountMosaicRestrictionTraits : public test::BaseAccountMosaicRestrictionTraits {
			static constexpr auto CreateValidator = CreateAccountMosaicRestrictionModificationActionsValidator;

			using NotificationType = model::ModifyAccountMosaicRestrictionNotification;
		};

		struct AccountOperationRestrictionTraits : public test::BaseAccountOperationRestrictionTraits {
			static constexpr auto CreateValidator = CreateAccountOperationRestrictionModificationActionsValidator;

			using NotificationType = model::ModifyAccountOperationRestrictionNotification;
		};

		template<typename TRestrictionValueTraits>
		std::vector<model::AccountRestrictionModification<typename TRestrictionValueTraits::UnresolvedValueType>> CreateModifications(
				const std::vector<model::AccountRestrictionModificationAction>& modificationActions) {
			std::vector<model::AccountRestrictionModification<typename TRestrictionValueTraits::UnresolvedValueType>> modifications;
			for (auto modificationAction : modificationActions)
				modifications.push_back({ modificationAction, TRestrictionValueTraits::RandomUnresolvedValue() });

			return modifications;
		}

		template<typename TRestrictionValueTraits>
		void AssertValidationResult(
				ValidationResult expectedResult,
				const std::vector<model::AccountRestrictionModificationAction>& modificationActions) {
			// Arrange:
			auto modifications = CreateModifications<TRestrictionValueTraits>(modificationActions);
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

		model::AccountRestrictionModificationAction CreateType(uint8_t value) {
			return static_cast<model::AccountRestrictionModificationAction>(value);
		}
	}

#define TRAITS_BASED_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_Address) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<AccountAddressRestrictionTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Mosaic) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<AccountMosaicRestrictionTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Operation) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<AccountOperationRestrictionTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	TRAITS_BASED_TEST(FailureWhenValidatingNotificationWithUnknownAccountRestrictionModificationAction) {
		AssertValidationResult<TTraits>(Failure_RestrictionAccount_Invalid_Modification_Action, { Add, Del, CreateType(0x02), Del });
		AssertValidationResult<TTraits>(Failure_RestrictionAccount_Invalid_Modification_Action, { CreateType(0x12) });
		AssertValidationResult<TTraits>(Failure_RestrictionAccount_Invalid_Modification_Action, { CreateType(0xFF), CreateType(0x12) });
	}

	TRAITS_BASED_TEST(SuccessWhenValidatingNotificationWithNoAccountRestrictionModificationActions) {
		AssertValidationResult<TTraits>(ValidationResult::Success, {});
	}

	TRAITS_BASED_TEST(SuccessWhenValidatingNotificationWithKnownAccountRestrictionModificationActions) {
		AssertValidationResult<TTraits>(ValidationResult::Success, { Add, Add, Del, Add, Del, Del });
	}
}}
