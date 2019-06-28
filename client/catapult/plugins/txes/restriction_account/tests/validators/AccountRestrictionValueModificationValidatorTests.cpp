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
#include "tests/test/plugins/ValidatorTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace validators {

#define TEST_CLASS AccountRestrictionValueModificationValidatorTests

	DEFINE_COMMON_VALIDATOR_TESTS(AccountAddressRestrictionValueModification,)
	DEFINE_COMMON_VALIDATOR_TESTS(AccountMosaicRestrictionValueModification,)
	DEFINE_COMMON_VALIDATOR_TESTS(AccountOperationRestrictionValueModification,)

	namespace {
		constexpr auto Add = model::AccountRestrictionModificationType::Add;
		constexpr auto Del = model::AccountRestrictionModificationType::Del;

		struct AccountAddressRestrictionTraits : public test::BaseAccountAddressRestrictionTraits {
			static constexpr auto CreateValidator = CreateAccountAddressRestrictionValueModificationValidator;

			using NotificationType = model::ModifyAccountAddressRestrictionValueNotification;

			static UnresolvedValueType ToUnresolved(const ValueType& value) {
				return extensions::CopyToUnresolvedAddress(value);
			}
		};

		struct AccountMosaicRestrictionTraits : public test::BaseAccountMosaicRestrictionTraits {
			static constexpr auto CreateValidator = CreateAccountMosaicRestrictionValueModificationValidator;

			using NotificationType = model::ModifyAccountMosaicRestrictionValueNotification;

			static UnresolvedValueType ToUnresolved(const ValueType& value) {
				return extensions::CastToUnresolvedMosaicId(value);
			}
		};

		struct AccountOperationRestrictionTraits : public test::BaseAccountOperationRestrictionTraits {
			static constexpr auto CreateValidator = CreateAccountOperationRestrictionValueModificationValidator;

			using NotificationType = model::ModifyAccountOperationRestrictionValueNotification;

			static UnresolvedValueType ToUnresolved(const ValueType& value) {
				return value;
			}
		};

		template<typename TRestrictionValueTraits>
		void RunValidator(
				ValidationResult expectedResult,
				cache::CatapultCache& cache,
				const typename TRestrictionValueTraits::NotificationType& notification) {
			// Arrange:
			auto pValidator = TRestrictionValueTraits::CreateValidator();

			// Act:
			auto result = test::ValidateNotification<typename TRestrictionValueTraits::NotificationType>(*pValidator, notification, cache);

			// Assert:
			EXPECT_EQ(expectedResult, result);
		}
	}

#define TRAITS_BASED_TEST(TEST_NAME) \
	template<typename TRestrictionValueTraits, typename TOperationTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_Address_Allow) { \
		TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<AccountAddressRestrictionTraits, test::AllowTraits>(); \
	} \
	TEST(TEST_CLASS, TEST_NAME##_Address_Block) { \
		TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<AccountAddressRestrictionTraits, test::BlockTraits>(); \
	} \
	TEST(TEST_CLASS, TEST_NAME##_Mosaic_Allow) { \
		TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<AccountMosaicRestrictionTraits, test::AllowTraits>(); \
	} \
	TEST(TEST_CLASS, TEST_NAME##_Mosaic_Block) { \
		TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<AccountMosaicRestrictionTraits, test::BlockTraits>(); \
	} \
	TEST(TEST_CLASS, TEST_NAME##_Operation_Allow) { \
		TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<AccountOperationRestrictionTraits, test::AllowTraits>(); \
	} \
	TEST(TEST_CLASS, TEST_NAME##_Operation_Block) { \
		TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<AccountOperationRestrictionTraits, test::BlockTraits>(); \
	} \
	template<typename TRestrictionValueTraits, typename TOperationTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	namespace {
		template<typename TRestrictionValueTraits, typename TOperationTraits>
		auto CreateNotification(
				const Key& key,
				const model::AccountRestrictionModification<typename TRestrictionValueTraits::UnresolvedValueType>& modification) {
			return test::CreateNotification<TRestrictionValueTraits, TOperationTraits>(key, modification);
		}

		template<typename TRestrictionValueTraits, typename TOperationTraits>
		auto CreateNotificationWithRandomKey(
				const Key&,
				const model::AccountRestrictionModification<typename TRestrictionValueTraits::UnresolvedValueType>& modification) {
			return test::CreateNotification<TRestrictionValueTraits, TOperationTraits>(test::GenerateRandomByteArray<Key>(), modification);
		}

		template<typename TRestrictionValueTraits, typename TOperationTraits, typename TCreateNotification, typename TModificationFactory>
		void AssertValidationResult(
				ValidationResult expectedResult,
				size_t numValues,
				TCreateNotification createNotification,
				TModificationFactory modificationFactory) {
			// Arrange:
			auto cache = test::AccountRestrictionCacheFactory::Create();
			auto key = test::GenerateRandomByteArray<Key>();
			auto values = test::GenerateUniqueRandomDataVector<typename TRestrictionValueTraits::ValueType>(numValues);
			test::PopulateCache<TRestrictionValueTraits, TOperationTraits>(cache, key, values);
			auto modification = modificationFactory(values);

			// Act:
			RunValidator<TRestrictionValueTraits>(expectedResult, cache, createNotification(key, modification));
		}
	}

	TRAITS_BASED_TEST(SuccessWhenAccountIsUnknown) {
		// Act + Assert:
		auto createNotification = CreateNotificationWithRandomKey<TRestrictionValueTraits, TOperationTraits>;
		constexpr auto Success = ValidationResult::Success;
		AssertValidationResult<TRestrictionValueTraits, TOperationTraits>(Success, 0, createNotification, [](const auto&) {
			using UnresolvedValue = typename TRestrictionValueTraits::UnresolvedValueType;
			using AccountRestrictionModification = model::AccountRestrictionModification<UnresolvedValue>;

			return AccountRestrictionModification{ Add, TRestrictionValueTraits::RandomUnresolvedValue() };
		});
	}

	TRAITS_BASED_TEST(FailureWhenAccountRestrictionAlreadyContainsValue_Add) {
		// Act + Assert:
		auto createNotification = CreateNotification<TRestrictionValueTraits, TOperationTraits>;
		constexpr auto Failure = Failure_RestrictionAccount_Modification_Not_Allowed;
		AssertValidationResult<TRestrictionValueTraits, TOperationTraits>(Failure, 3, createNotification, [](const auto& values) {
			using UnresolvedValue = typename TRestrictionValueTraits::UnresolvedValueType;
			using AccountRestrictionModification = model::AccountRestrictionModification<UnresolvedValue>;

			return AccountRestrictionModification{ Add, TRestrictionValueTraits::Unresolve(values[1]) };
		});
	}

	TRAITS_BASED_TEST(SuccessWhenAccountRestrictionDoesNotContainValue_Add) {
		// Act + Assert:
		auto createNotification = CreateNotification<TRestrictionValueTraits, TOperationTraits>;
		constexpr auto Success = ValidationResult::Success;
		AssertValidationResult<TRestrictionValueTraits, TOperationTraits>(Success, 3, createNotification, [](const auto& values) {
			using UnresolvedValue = typename TRestrictionValueTraits::UnresolvedValueType;
			using AccountRestrictionModification = model::AccountRestrictionModification<UnresolvedValue>;

			return AccountRestrictionModification{ Add, TRestrictionValueTraits::ToUnresolved(test::CreateRandomUniqueValue(values)) };
		});
	}

	TRAITS_BASED_TEST(FailureWhenAccountRestrictionDoesNotContainValue_Del) {
		// Act + Assert:
		auto createNotification = CreateNotification<TRestrictionValueTraits, TOperationTraits>;
		constexpr auto Failure = Failure_RestrictionAccount_Modification_Not_Allowed;
		AssertValidationResult<TRestrictionValueTraits, TOperationTraits>(Failure, 3, createNotification, [](const auto& values) {
			using UnresolvedValue = typename TRestrictionValueTraits::UnresolvedValueType;
			using AccountRestrictionModification = model::AccountRestrictionModification<UnresolvedValue>;

			return AccountRestrictionModification{ Del, TRestrictionValueTraits::ToUnresolved(test::CreateRandomUniqueValue(values)) };
		});
	}

	TRAITS_BASED_TEST(SuccessWhenAccountRestrictionContainsValue_Del) {
		// Act + Assert:
		auto createNotification = CreateNotification<TRestrictionValueTraits, TOperationTraits>;
		constexpr auto Success = ValidationResult::Success;
		AssertValidationResult<TRestrictionValueTraits, TOperationTraits>(Success, 3, createNotification, [](const auto& values) {
			using UnresolvedValue = typename TRestrictionValueTraits::UnresolvedValueType;
			using AccountRestrictionModification = model::AccountRestrictionModification<UnresolvedValue>;

			return AccountRestrictionModification{ Del, TRestrictionValueTraits::Unresolve(values[2]) };
		});
	}

	TRAITS_BASED_TEST(FailureWhenOperationConflictsExistingAccountRestrictionAndValuesAreNotEmpty_Add) {
		// Act + Assert: restriction is configured as "Allow" / "Block" but notification operation type is "Block" / "Allow"
		auto createNotification = test::CreateNotificationWithOppositeOperation<TRestrictionValueTraits, TOperationTraits>;
		constexpr auto Failure = Failure_RestrictionAccount_Modification_Not_Allowed;
		AssertValidationResult<TRestrictionValueTraits, TOperationTraits>(Failure, 3, createNotification, [](const auto&) {
			using UnresolvedValue = typename TRestrictionValueTraits::UnresolvedValueType;
			using AccountRestrictionModification = model::AccountRestrictionModification<UnresolvedValue>;

			return AccountRestrictionModification{ Add, TRestrictionValueTraits::RandomUnresolvedValue() };
		});
	}

	TRAITS_BASED_TEST(FailureWhenOperationConflictsExistingAccountRestrictionAndValuesAreNotEmpty_Del) {
		// Act + Assert: restriction is configured as "Allow" / "Block" but notification operation type is "Block" / "Allow"
		auto createNotification = test::CreateNotificationWithOppositeOperation<TRestrictionValueTraits, TOperationTraits>;
		constexpr auto Failure = Failure_RestrictionAccount_Modification_Not_Allowed;
		AssertValidationResult<TRestrictionValueTraits, TOperationTraits>(Failure, 3, createNotification, [](const auto& values) {
			using UnresolvedValue = typename TRestrictionValueTraits::UnresolvedValueType;
			using AccountRestrictionModification = model::AccountRestrictionModification<UnresolvedValue>;

			return AccountRestrictionModification{ Del, TRestrictionValueTraits::Unresolve(values[0]) };
		});
	}

	TRAITS_BASED_TEST(SuccessWhenOperationConflictsExistingAccountRestrictionAndValuesAreEmpty) {
		// Act + Assert:
		auto createNotification = test::CreateNotificationWithOppositeOperation<TRestrictionValueTraits, TOperationTraits>;
		constexpr auto Success = ValidationResult::Success;
		AssertValidationResult<TRestrictionValueTraits, TOperationTraits>(Success, 0, createNotification, [](const auto&) {
			using UnresolvedValue = typename TRestrictionValueTraits::UnresolvedValueType;
			using AccountRestrictionModification = model::AccountRestrictionModification<UnresolvedValue>;

			return AccountRestrictionModification{ Add, TRestrictionValueTraits::RandomUnresolvedValue() };
		});
	}
}}
