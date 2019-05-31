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
#include "tests/test/PropertyCacheTestUtils.h"
#include "tests/test/core/AddressTestUtils.h"
#include "tests/test/plugins/ValidatorTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace validators {

#define TEST_CLASS PropertyModificationTypesValidatorTests

	DEFINE_COMMON_VALIDATOR_TESTS(AddressPropertyModificationTypes,)
	DEFINE_COMMON_VALIDATOR_TESTS(MosaicPropertyModificationTypes,)
	DEFINE_COMMON_VALIDATOR_TESTS(TransactionTypePropertyModificationTypes,)

	namespace {
		constexpr auto Add = model::PropertyModificationType::Add;
		constexpr auto Del = model::PropertyModificationType::Del;

		struct AddressPropertyTraits : public test::BaseAddressPropertyTraits {
			static constexpr auto CreateValidator = CreateAddressPropertyModificationTypesValidator;

			using NotificationType = model::ModifyAddressPropertyNotification;
		};

		struct MosaicPropertyTraits : public test::BaseMosaicPropertyTraits {
			static constexpr auto CreateValidator = CreateMosaicPropertyModificationTypesValidator;

			using NotificationType = model::ModifyMosaicPropertyNotification;
		};

		struct TransactionTypePropertyTraits : public test::BaseTransactionTypePropertyTraits {
			static constexpr auto CreateValidator = CreateTransactionTypePropertyModificationTypesValidator;

			using NotificationType = model::ModifyTransactionTypePropertyNotification;
		};

		template<typename TPropertyValueTraits>
		std::vector<model::PropertyModification<typename TPropertyValueTraits::UnresolvedValueType>> CreateModifications(
				const std::vector<model::PropertyModificationType>& modificationTypes) {
			std::vector<model::PropertyModification<typename TPropertyValueTraits::UnresolvedValueType>> modifications;
			for (auto modificationType : modificationTypes)
				modifications.push_back({ modificationType, TPropertyValueTraits::RandomUnresolvedValue() });

			return modifications;
		}

		template<typename TPropertyValueTraits>
		void AssertValidationResult(
				ValidationResult expectedResult,
				const std::vector<model::PropertyModificationType>& modificationTypes) {
			// Arrange:
			auto modifications = CreateModifications<TPropertyValueTraits>(modificationTypes);
			typename TPropertyValueTraits::NotificationType notification(
					test::GenerateRandomByteArray<Key>(),
					TPropertyValueTraits::Property_Type,
					static_cast<uint8_t>(modifications.size()),
					modifications.data());
			auto pValidator = TPropertyValueTraits::CreateValidator();

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification);

			// Assert:
			EXPECT_EQ(expectedResult, result);
		}

		model::PropertyModificationType CreateType(uint8_t value) {
			return static_cast<model::PropertyModificationType>(value);
		}
	}

#define TRAITS_BASED_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_Address) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<AddressPropertyTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Mosaic) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<MosaicPropertyTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_TransactionType) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<TransactionTypePropertyTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	TRAITS_BASED_TEST(FailureWhenValidatingNotificationWithUnknownPropertyModificationType) {
		// Assert:
		AssertValidationResult<TTraits>(Failure_Property_Modification_Type_Invalid, { Add, Del, CreateType(0x02), Del });
		AssertValidationResult<TTraits>(Failure_Property_Modification_Type_Invalid, { CreateType(0x12) });
		AssertValidationResult<TTraits>(Failure_Property_Modification_Type_Invalid, { CreateType(0xFF), CreateType(0x12) });
	}

	TRAITS_BASED_TEST(SuccessWhenValidatingNotificationWithNoPropertyModificationTypes) {
		// Assert:
		AssertValidationResult<TTraits>(ValidationResult::Success, {});
	}

	TRAITS_BASED_TEST(SuccessWhenValidatingNotificationWithKnownPropertyModificationTypes) {
		// Assert:
		AssertValidationResult<TTraits>(ValidationResult::Success, { Add, Add, Del, Add, Del, Del });
	}
}}
