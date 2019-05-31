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

#define TEST_CLASS PropertyRedundantModificationValidatorTests

	DEFINE_COMMON_VALIDATOR_TESTS(AddressPropertyRedundantModification,)
	DEFINE_COMMON_VALIDATOR_TESTS(MosaicPropertyRedundantModification,)
	DEFINE_COMMON_VALIDATOR_TESTS(TransactionTypePropertyRedundantModification,)

	namespace {
		constexpr auto Add = model::PropertyModificationType::Add;
		constexpr auto Del = model::PropertyModificationType::Del;

		enum class CacheSeed { No, Yes};

		struct AddressPropertyTraits : public test::BaseAddressPropertyTraits {
			static constexpr auto CreateValidator = CreateAddressPropertyRedundantModificationValidator;

			using NotificationType = model::ModifyAddressPropertyNotification;
		};

		struct MosaicPropertyTraits : public test::BaseMosaicPropertyTraits {
			static constexpr auto CreateValidator = CreateMosaicPropertyRedundantModificationValidator;

			using NotificationType = model::ModifyMosaicPropertyNotification;
		};

		struct TransactionTypePropertyTraits : public test::BaseTransactionTypePropertyTraits {
			static constexpr auto CreateValidator = CreateTransactionTypePropertyRedundantModificationValidator;

			using NotificationType = model::ModifyTransactionTypePropertyNotification;
		};

		template<typename TPropertyValueTraits, typename TModificationsFactory>
		void AssertValidationResult(ValidationResult expectedResult, CacheSeed cacheSeed, TModificationsFactory modificationsFactory) {
			// Arrange:
			auto values = test::GenerateUniqueRandomDataVector<typename TPropertyValueTraits::UnresolvedValueType>(5);
			auto modifications = modificationsFactory(values);
			typename TPropertyValueTraits::NotificationType notification(
					test::GenerateRandomByteArray<Key>(),
					TPropertyValueTraits::Property_Type,
					static_cast<uint8_t>(modifications.size()),
					modifications.data());
			auto cache = test::PropertyCacheFactory::Create();
			if (CacheSeed::Yes == cacheSeed) {
				auto address = model::PublicKeyToAddress(notification.Key, model::NetworkIdentifier::Zero);
				auto accountProperties = state::AccountProperties(address);
				accountProperties.property(TPropertyValueTraits::Property_Type).allow({ Add, state::ToVector(values[1]) });
				auto delta = cache.createDelta();
				auto& propertyDelta = delta.template sub<cache::PropertyCache>();
				propertyDelta.insert(accountProperties);
				cache.commit(Height(1));
			}

			auto pValidator = TPropertyValueTraits::CreateValidator();

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification, cache);

			// Assert:
			EXPECT_EQ(expectedResult, result);
		}

		template<typename TPropertyValueTraits, typename TModificationsFactory>
		void AssertValidationResult(ValidationResult expectedResult, TModificationsFactory modificationsFactory) {
			AssertValidationResult<TPropertyValueTraits>(expectedResult, CacheSeed::No, modificationsFactory);
		}
	}

#define TRAITS_BASED_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_Address) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<AddressPropertyTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Mosaic) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<MosaicPropertyTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_TransactionType) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<TransactionTypePropertyTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	TRAITS_BASED_TEST(FailureWhenValidatingNotificationWithRedundantAdds) {
		// Assert:
		AssertValidationResult<TTraits>(Failure_Property_Modification_Redundant, [](const auto& values) {
			return std::vector<model::PropertyModification<typename TTraits::UnresolvedValueType>>{
				{ Add, values[2] },
				{ Add, test::CreateRandomUniqueValue(values) },
				{ Add, values[0] },
				{ Add, values[2] }
			};
		});
	}

	TRAITS_BASED_TEST(FailureWhenValidatingNotificationWithRedundantDels) {
		// Assert:
		AssertValidationResult<TTraits>(Failure_Property_Modification_Redundant, [](const auto& values) {
			return std::vector<model::PropertyModification<typename TTraits::UnresolvedValueType>>{
				{ Del, values[2] },
				{ Add, test::CreateRandomUniqueValue(values) },
				{ Add, values[0] },
				{ Del, values[2] }
			};
		});
	}

	TRAITS_BASED_TEST(FailureWhenValidatingNotificationWithRedundantAddAndDelete) {
		// Assert:
		AssertValidationResult<TTraits>(Failure_Property_Modification_Redundant, [](const auto& values) {
			return std::vector<model::PropertyModification<typename TTraits::UnresolvedValueType>>{
				{ Add, values[2] },
				{ Add, test::CreateRandomUniqueValue(values) },
				{ Add, values[0] },
				{ Del, values[2] }
			};
		});
	}

	TRAITS_BASED_TEST(FailureWhenValidatingNotificationWithUnknownAddressAndDelete) {
		// Assert: account properties is not found in cache, so Del operation triggers failure
		AssertValidationResult<TTraits>(Failure_Property_Modification_Not_Allowed, [](const auto& values) {
			return std::vector<model::PropertyModification<typename TTraits::UnresolvedValueType>>{
				{ Add, values[2] },
				{ Del, test::CreateRandomUniqueValue(values) },
				{ Add, values[0] }
			};
		});
	}

	TRAITS_BASED_TEST(SuccessWhenValidatingNotificationWithUnknownAddressAndNoRedundantOperationAndNoDelete) {
		// Assert:
		AssertValidationResult<TTraits>(ValidationResult::Success, [](const auto& values) {
			return std::vector<model::PropertyModification<typename TTraits::UnresolvedValueType>>{
				{ Add, values[2] },
				{ Add, values[1] },
				{ Add, values[0] }
			};
		});
	}

	TRAITS_BASED_TEST(SuccessWhenValidatingNotificationWithKnownAddressAndNoRedundantOperationAndValidDelete) {
		// Assert:
		AssertValidationResult<TTraits>(ValidationResult::Success, CacheSeed::Yes, [](const auto& values) {
			return std::vector<model::PropertyModification<typename TTraits::UnresolvedValueType>>{
				{ Add, values[2] },
				{ Add, test::CreateRandomUniqueValue(values) },
				{ Del, values[1] },
				{ Add, values[0] }
			};
		});
	}
}}
