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

#define TEST_CLASS AccountRestrictionRedundantModificationValidatorTests

	DEFINE_COMMON_VALIDATOR_TESTS(AccountAddressRestrictionRedundantModification,)
	DEFINE_COMMON_VALIDATOR_TESTS(AccountMosaicRestrictionRedundantModification,)
	DEFINE_COMMON_VALIDATOR_TESTS(AccountOperationRestrictionRedundantModification,)

	namespace {
		constexpr auto Add = model::AccountRestrictionModificationAction::Add;
		constexpr auto Del = model::AccountRestrictionModificationAction::Del;

		enum class CacheSeed { No, Yes};

		struct AccountAddressRestrictionTraits : public test::BaseAccountAddressRestrictionTraits {
			static constexpr auto CreateValidator = CreateAccountAddressRestrictionRedundantModificationValidator;

			using NotificationType = model::ModifyAccountAddressRestrictionNotification;
		};

		struct AccountMosaicRestrictionTraits : public test::BaseAccountMosaicRestrictionTraits {
			static constexpr auto CreateValidator = CreateAccountMosaicRestrictionRedundantModificationValidator;

			using NotificationType = model::ModifyAccountMosaicRestrictionNotification;
		};

		struct AccountOperationRestrictionTraits : public test::BaseAccountOperationRestrictionTraits {
			static constexpr auto CreateValidator = CreateAccountOperationRestrictionRedundantModificationValidator;

			using NotificationType = model::ModifyAccountOperationRestrictionNotification;
		};

		template<typename TRestrictionValueTraits, typename TModificationsFactory>
		void AssertValidationResult(ValidationResult expectedResult, CacheSeed cacheSeed, TModificationsFactory modificationsFactory) {
			// Arrange:
			auto values = test::GenerateUniqueRandomDataVector<typename TRestrictionValueTraits::UnresolvedValueType>(5);
			auto modifications = modificationsFactory(values);
			typename TRestrictionValueTraits::NotificationType notification(
					test::GenerateRandomByteArray<Key>(),
					TRestrictionValueTraits::Restriction_Type,
					static_cast<uint8_t>(modifications.size()),
					modifications.data());
			auto cache = test::AccountRestrictionCacheFactory::Create();
			if (CacheSeed::Yes == cacheSeed) {
				auto address = model::PublicKeyToAddress(notification.Key, model::NetworkIdentifier::Zero);
				auto restrictions = state::AccountRestrictions(address);
				restrictions.restriction(TRestrictionValueTraits::Restriction_Type).allow({ Add, state::ToVector(values[1]) });
				auto delta = cache.createDelta();
				auto& restrictionCacheDelta = delta.template sub<cache::AccountRestrictionCache>();
				restrictionCacheDelta.insert(restrictions);
				cache.commit(Height(1));
			}

			auto pValidator = TRestrictionValueTraits::CreateValidator();

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification, cache);

			// Assert:
			EXPECT_EQ(expectedResult, result);
		}

		template<typename TRestrictionValueTraits, typename TModificationsFactory>
		void AssertValidationResult(ValidationResult expectedResult, TModificationsFactory modificationsFactory) {
			AssertValidationResult<TRestrictionValueTraits>(expectedResult, CacheSeed::No, modificationsFactory);
		}
	}

#define TRAITS_BASED_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_Address) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<AccountAddressRestrictionTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Mosaic) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<AccountMosaicRestrictionTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Operation) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<AccountOperationRestrictionTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	TRAITS_BASED_TEST(FailureWhenValidatingNotificationWithRedundantAdds) {
		AssertValidationResult<TTraits>(Failure_RestrictionAccount_Redundant_Modification, [](const auto& values) {
			return std::vector<model::AccountRestrictionModification<typename TTraits::UnresolvedValueType>>{
				{ Add, values[2] },
				{ Add, test::CreateRandomUniqueValue(values) },
				{ Add, values[0] },
				{ Add, values[2] }
			};
		});
	}

	TRAITS_BASED_TEST(FailureWhenValidatingNotificationWithRedundantDels) {
		AssertValidationResult<TTraits>(Failure_RestrictionAccount_Redundant_Modification, [](const auto& values) {
			return std::vector<model::AccountRestrictionModification<typename TTraits::UnresolvedValueType>>{
				{ Del, values[2] },
				{ Add, test::CreateRandomUniqueValue(values) },
				{ Add, values[0] },
				{ Del, values[2] }
			};
		});
	}

	TRAITS_BASED_TEST(FailureWhenValidatingNotificationWithRedundantAddAndDelete) {
		AssertValidationResult<TTraits>(Failure_RestrictionAccount_Redundant_Modification, [](const auto& values) {
			return std::vector<model::AccountRestrictionModification<typename TTraits::UnresolvedValueType>>{
				{ Add, values[2] },
				{ Add, test::CreateRandomUniqueValue(values) },
				{ Add, values[0] },
				{ Del, values[2] }
			};
		});
	}

	TRAITS_BASED_TEST(FailureWhenValidatingNotificationWithUnknownAddressAndDelete) {
		// Assert: account restrictions is not found in cache, so Del operation triggers failure
		AssertValidationResult<TTraits>(Failure_RestrictionAccount_Invalid_Modification, [](const auto& values) {
			return std::vector<model::AccountRestrictionModification<typename TTraits::UnresolvedValueType>>{
				{ Add, values[2] },
				{ Del, test::CreateRandomUniqueValue(values) },
				{ Add, values[0] }
			};
		});
	}

	TRAITS_BASED_TEST(SuccessWhenValidatingNotificationWithUnknownAddressAndNoRedundantOperationAndNoDelete) {
		AssertValidationResult<TTraits>(ValidationResult::Success, [](const auto& values) {
			return std::vector<model::AccountRestrictionModification<typename TTraits::UnresolvedValueType>>{
				{ Add, values[2] },
				{ Add, values[1] },
				{ Add, values[0] }
			};
		});
	}

	TRAITS_BASED_TEST(SuccessWhenValidatingNotificationWithKnownAddressAndNoRedundantOperationAndValidDelete) {
		AssertValidationResult<TTraits>(ValidationResult::Success, CacheSeed::Yes, [](const auto& values) {
			return std::vector<model::AccountRestrictionModification<typename TTraits::UnresolvedValueType>>{
				{ Add, values[2] },
				{ Add, test::CreateRandomUniqueValue(values) },
				{ Del, values[1] },
				{ Add, values[0] }
			};
		});
	}
}}
