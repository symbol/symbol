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
#include "tests/test/plugins/ValidatorTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace validators {

#define TEST_CLASS PropertyValueModificationValidatorTests

	DEFINE_COMMON_VALIDATOR_TESTS(AddressPropertyValueModification,)
	DEFINE_COMMON_VALIDATOR_TESTS(MosaicPropertyValueModification,)
	DEFINE_COMMON_VALIDATOR_TESTS(TransactionTypePropertyValueModification,)

	namespace {
		constexpr auto Add = model::PropertyModificationType::Add;
		constexpr auto Del = model::PropertyModificationType::Del;

		struct AddressPropertyTraits : public test::BaseAddressPropertyTraits {
			static constexpr auto CreateValidator = CreateAddressPropertyValueModificationValidator;

			using NotificationType = model::ModifyAddressPropertyValueNotification;

			static UnresolvedValueType ToUnresolved(const ValueType& value) {
				return extensions::CopyToUnresolvedAddress(value);
			}
		};

		struct MosaicPropertyTraits : public test::BaseMosaicPropertyTraits {
			static constexpr auto CreateValidator = CreateMosaicPropertyValueModificationValidator;

			using NotificationType = model::ModifyMosaicPropertyValueNotification;

			static UnresolvedValueType ToUnresolved(const ValueType& value) {
				return extensions::CastToUnresolvedMosaicId(value);
			}
		};

		struct TransactionTypePropertyTraits : public test::BaseTransactionTypePropertyTraits {
			static constexpr auto CreateValidator = CreateTransactionTypePropertyValueModificationValidator;

			using NotificationType = model::ModifyTransactionTypePropertyValueNotification;

			static UnresolvedValueType ToUnresolved(const ValueType& value) {
				return value;
			}
		};

		template<typename TPropertyValueTraits>
		void RunValidator(
				ValidationResult expectedResult,
				cache::CatapultCache& cache,
				const typename TPropertyValueTraits::NotificationType& notification) {
			// Arrange:
			auto pValidator = TPropertyValueTraits::CreateValidator();

			// Act:
			auto result = test::ValidateNotification<typename TPropertyValueTraits::NotificationType>(*pValidator, notification, cache);

			// Assert:
			EXPECT_EQ(expectedResult, result);
		}
	}

#define TRAITS_BASED_TEST(TEST_NAME) \
	template<typename TPropertyValueTraits, typename TOperationTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_Address_Allow) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<AddressPropertyTraits, test::AllowTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Address_Block) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<AddressPropertyTraits, test::BlockTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Mosaic_Allow) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<MosaicPropertyTraits, test::AllowTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Mosaic_Block) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<MosaicPropertyTraits, test::BlockTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_TransactionType_Allow) { \
		TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<TransactionTypePropertyTraits, test::AllowTraits>(); \
	} \
	TEST(TEST_CLASS, TEST_NAME##_TransactionType_Block) { \
		TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<TransactionTypePropertyTraits, test::BlockTraits>(); \
	} \
	template<typename TPropertyValueTraits, typename TOperationTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	namespace {
		template<typename TPropertyValueTraits, typename TOperationTraits>
		auto CreateNotification(
				const Key& key,
				const model::PropertyModification<typename TPropertyValueTraits::UnresolvedValueType>& modification) {
			return test::CreateNotification<TPropertyValueTraits, TOperationTraits>(key, modification);
		}

		template<typename TPropertyValueTraits, typename TOperationTraits>
		auto CreateNotificationWithRandomKey(
				const Key&,
				const model::PropertyModification<typename TPropertyValueTraits::UnresolvedValueType>& modification) {
			return test::CreateNotification<TPropertyValueTraits, TOperationTraits>(test::GenerateRandomData<Key_Size>(), modification);
		}

		template<typename TPropertyValueTraits, typename TOperationTraits, typename TCreateNotification, typename TModificationFactory>
		void AssertValidationResult(
				ValidationResult expectedResult,
				size_t numValues,
				TCreateNotification createNotification,
				TModificationFactory modificationFactory) {
			// Arrange:
			auto cache = test::PropertyCacheFactory::Create();
			auto key = test::GenerateRandomData<Key_Size>();
			auto values = test::CreateRandomUniqueValues<TPropertyValueTraits>(numValues);
			test::PopulateCache<TPropertyValueTraits, TOperationTraits>(cache, key, values);
			auto modification = modificationFactory(values);

			// Act:
			RunValidator<TPropertyValueTraits>(expectedResult, cache, createNotification(key, modification));
		}
	}

	TRAITS_BASED_TEST(SuccessWhenAccountIsUnknown) {
		// Act + Assert:
		auto createNotification = CreateNotificationWithRandomKey<TPropertyValueTraits, TOperationTraits>;
		constexpr auto Success = ValidationResult::Success;
		AssertValidationResult<TPropertyValueTraits, TOperationTraits>(Success, 0, createNotification, [](const auto&) {
			using PropertyModification = model::PropertyModification<typename TPropertyValueTraits::UnresolvedValueType>;
			return PropertyModification{ Add, TPropertyValueTraits::RandomUnresolvedValue() };
		});
	}

	TRAITS_BASED_TEST(FailureWhenAccountPropertyAlreadyContainsValue_Add) {
		// Act + Assert:
		auto createNotification = CreateNotification<TPropertyValueTraits, TOperationTraits>;
		constexpr auto Failure = Failure_Property_Modification_Not_Allowed;
		AssertValidationResult<TPropertyValueTraits, TOperationTraits>(Failure, 3, createNotification, [](const auto& values) {
			using PropertyModification = model::PropertyModification<typename TPropertyValueTraits::UnresolvedValueType>;
			return PropertyModification{ Add, TPropertyValueTraits::Unresolve(values[1]) };
		});
	}

	TRAITS_BASED_TEST(SuccessWhenAccountPropertyDoesNotContainValue_Add) {
		// Act + Assert:
		auto createNotification = CreateNotification<TPropertyValueTraits, TOperationTraits>;
		constexpr auto Success = ValidationResult::Success;
		AssertValidationResult<TPropertyValueTraits, TOperationTraits>(Success, 3, createNotification, [](const auto& values) {
			using PropertyModification = model::PropertyModification<typename TPropertyValueTraits::UnresolvedValueType>;
			auto transform = TPropertyValueTraits::ToUnresolved;
			return PropertyModification{ Add, test::CreateDifferentValue<TPropertyValueTraits>(values, transform) };
		});
	}

	TRAITS_BASED_TEST(FailureWhenAccountPropertyDoesNotContainValue_Del) {
		// Act + Assert:
		auto createNotification = CreateNotification<TPropertyValueTraits, TOperationTraits>;
		constexpr auto Failure = Failure_Property_Modification_Not_Allowed;
		AssertValidationResult<TPropertyValueTraits, TOperationTraits>(Failure, 3, createNotification, [](const auto& values) {
			using PropertyModification = model::PropertyModification<typename TPropertyValueTraits::UnresolvedValueType>;
			auto transform = TPropertyValueTraits::ToUnresolved;
			return PropertyModification{ Del, test::CreateDifferentValue<TPropertyValueTraits>(values, transform) };
		});
	}

	TRAITS_BASED_TEST(SuccessWhenAccountPropertyContainsValue_Del) {
		// Act + Assert:
		auto createNotification = CreateNotification<TPropertyValueTraits, TOperationTraits>;
		constexpr auto Success = ValidationResult::Success;
		AssertValidationResult<TPropertyValueTraits, TOperationTraits>(Success, 3, createNotification, [](const auto& values) {
			using PropertyModification = model::PropertyModification<typename TPropertyValueTraits::UnresolvedValueType>;
			return PropertyModification{ Del, TPropertyValueTraits::Unresolve(values[2]) };
		});
	}

	TRAITS_BASED_TEST(FailureWhenOperationConflictsExistingAccountPropertyAndValuesAreNotEmpty_Add) {
		// Act + Assert: property is configured as "Allow" / "Block" but notification operation type is "Block" / "Allow"
		auto createNotification = test::CreateNotificationWithOppositeOperation<TPropertyValueTraits, TOperationTraits>;
		constexpr auto Failure = Failure_Property_Modification_Not_Allowed;
		AssertValidationResult<TPropertyValueTraits, TOperationTraits>(Failure, 3, createNotification, [](const auto&) {
			using PropertyModification = model::PropertyModification<typename TPropertyValueTraits::UnresolvedValueType>;
			return PropertyModification{ Add, TPropertyValueTraits::RandomUnresolvedValue() };
		});
	}

	TRAITS_BASED_TEST(FailureWhenOperationConflictsExistingAccountPropertyAndValuesAreNotEmpty_Del) {
		// Act + Assert: property is configured as "Allow" / "Block" but notification operation type is "Block" / "Allow"
		auto createNotification = test::CreateNotificationWithOppositeOperation<TPropertyValueTraits, TOperationTraits>;
		constexpr auto Failure = Failure_Property_Modification_Not_Allowed;
		AssertValidationResult<TPropertyValueTraits, TOperationTraits>(Failure, 3, createNotification, [](const auto& values) {
			using PropertyModification = model::PropertyModification<typename TPropertyValueTraits::UnresolvedValueType>;
			return PropertyModification{ Del, TPropertyValueTraits::Unresolve(values[0]) };
		});
	}

	TRAITS_BASED_TEST(SuccessWhenOperationConflictsExistingAccountPropertyAndValuesAreEmpty) {
		// Act + Assert:
		auto createNotification = test::CreateNotificationWithOppositeOperation<TPropertyValueTraits, TOperationTraits>;
		constexpr auto Success = ValidationResult::Success;
		AssertValidationResult<TPropertyValueTraits, TOperationTraits>(Success, 0, createNotification, [](const auto&) {
			using PropertyModification = model::PropertyModification<typename TPropertyValueTraits::UnresolvedValueType>;
			return PropertyModification{ Add, TPropertyValueTraits::RandomUnresolvedValue() };
		});
	}
}}
