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
#include "catapult/cache/CatapultCache.h"
#include "catapult/cache_core/AccountStateCache.h"
#include "catapult/state/CatapultState.h"
#include "tests/test/cache/BalanceTransferTestUtils.h"
#include "tests/test/cache/CacheTestUtils.h"
#include "tests/test/core/AddressTestUtils.h"
#include "tests/test/plugins/ValidatorTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace validators {

	DEFINE_COMMON_VALIDATOR_TESTS(BalanceTransfer,)
	DEFINE_COMMON_VALIDATOR_TESTS(BalanceDebit,)

#define TEST_CLASS BalanceValidatorTests // used to generate unique function names in macros
#define TRANSFER_TEST_CLASS BalanceTransferValidatorTests
#define DEBIT_TEST_CLASS BalanceDebitValidatorTests

	namespace {
		constexpr auto Currency_Mosaic_Id = MosaicId(1234);
		constexpr auto Default_Dynamic_Fee_Multiplier = BlockFeeMultiplier(117);

		// region adjustment traits

		struct BalanceGreaterTraits {
			static constexpr auto Expected_Result = ValidationResult::Success;

			static constexpr Amount Adjust(Amount amount) {
				return amount + Amount(1234);
			}
		};

		struct BalanceEqualTraits {
			static constexpr auto Expected_Result = ValidationResult::Success;

			static constexpr Amount Adjust(Amount amount) {
				return amount;
			}
		};

		struct BalanceLessTraits {
			static constexpr auto Expected_Result = Failure_Core_Insufficient_Balance;

			static Amount Adjust(Amount amount) {
				return amount - Amount(1);
			}
		};

		// endregion

		// region transfer / debit traits

		struct TransferStaticTraits {
			static ValidationResult Validate(const model::BalanceTransferNotification& notification, const ValidatorContext& context) {
				auto pValidator = CreateBalanceTransferValidator();
				return test::ValidateNotification(*pValidator, notification, context);
			}
		};

		struct TransferDynamicTraits {
			static ValidationResult Validate(const model::BalanceTransferNotification& notification, const ValidatorContext& context) {
				auto dynamicNotification = model::BalanceTransferNotification(
						notification.Sender,
						notification.Recipient,
						notification.MosaicId,
						Amount(notification.Amount.unwrap() / context.Cache.dependentState().DynamicFeeMultiplier.unwrap()),
						model::BalanceTransferNotification::AmountType::Dynamic);

				auto pValidator = CreateBalanceTransferValidator();
				return test::ValidateNotification(*pValidator, dynamicNotification, context);
			}
		};

		struct DebitTraits {
			static ValidationResult Validate(const model::BalanceTransferNotification& notification, const ValidatorContext& context) {
				auto pValidator = CreateBalanceDebitValidator();

				// map transfer notification to a debit notification
				auto debitNotification = model::BalanceDebitNotification(notification.Sender, notification.MosaicId, notification.Amount);

				return test::ValidateNotification(*pValidator, debitNotification, context);
			}
		};

		// endregion

		void SetDynamicFeeMultiplier(cache::CatapultCache& cache, BlockFeeMultiplier dynamicFeeMultiplier) {
			auto cacheDelta = cache.createDelta();
			cacheDelta.dependentState().DynamicFeeMultiplier = dynamicFeeMultiplier;
			cache.commit(Height());
		}

		template<typename TTraits>
		void AssertValidationResult(
				ValidationResult expectedResult,
				cache::CatapultCache& cache,
				const model::BalanceTransferNotification& notification) {
			// Arrange:
			SetDynamicFeeMultiplier(cache, Default_Dynamic_Fee_Multiplier);

			auto cacheView = cache.createView();
			auto readOnlyCache = cacheView.toReadOnly();
			auto context = test::CreateValidatorContext(Height(1), readOnlyCache);

			// Act:
			auto result = TTraits::Validate(notification, context);

			// Assert:
			EXPECT_EQ(expectedResult, result);
		}
	}

	// region traits

#define TRANSFER_OR_DEBIT_TEST(TEST_NAME) \
	template<typename TValidatorTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TRANSFER_TEST_CLASS, TEST_NAME##_Static) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<TransferStaticTraits>(); } \
	TEST(TRANSFER_TEST_CLASS, TEST_NAME##_Dynamic) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<TransferDynamicTraits>(); } \
	TEST(DEBIT_TEST_CLASS, TEST_NAME) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<DebitTraits>(); } \
	template<typename TValidatorTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

#define VARIABLE_BALANCE_TEST_GROUP(GROUP_TEST_CLASS, TEST_NAME, TEST_POSTFIX, TRAITS_NAME) \
	TEST(GROUP_TEST_CLASS, SuccessWhenBalanceIsGreater_##TEST_NAME##TEST_POSTFIX) { \
		TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<TRAITS_NAME, BalanceGreaterTraits>(); \
	} \
	TEST(GROUP_TEST_CLASS, SuccessWhenBalanceIsExact_##TEST_NAME##TEST_POSTFIX) { \
		TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<TRAITS_NAME, BalanceEqualTraits>(); \
	} \
	TEST(GROUP_TEST_CLASS, FailureWhenBalanceIsTooSmall_##TEST_NAME##TEST_POSTFIX) { \
		TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<TRAITS_NAME, BalanceLessTraits>(); \
	}

#define VARIABLE_BALANCE_TEST(TEST_NAME) \
	template<typename TValidatorTraits, typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	VARIABLE_BALANCE_TEST_GROUP(TRANSFER_TEST_CLASS, TEST_NAME, _Static, TransferStaticTraits) \
	VARIABLE_BALANCE_TEST_GROUP(TRANSFER_TEST_CLASS, TEST_NAME, _Dynamic, TransferDynamicTraits) \
	VARIABLE_BALANCE_TEST_GROUP(DEBIT_TEST_CLASS, TEST_NAME, , DebitTraits) \
	template<typename TValidatorTraits, typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	// endregion

	// region edge case tests

	TRANSFER_OR_DEBIT_TEST(FailureWhenTransactionSignerIsUnknown) {
		// Arrange: do not register the signer with the cache
		auto sender = test::GenerateRandomByteArray<Address>();
		auto recipient = test::GenerateRandomUnresolvedAddress();
		auto notification = model::BalanceTransferNotification(sender, recipient, test::UnresolveXor(Currency_Mosaic_Id), Amount(0));
		auto cache = test::CreateEmptyCatapultCache();

		// Assert:
		AssertValidationResult<TValidatorTraits>(Failure_Core_Insufficient_Balance, cache, notification);
	}

	TEST(TRANSFER_TEST_CLASS, FailureWhenEffectiveBalanceOverflows_Dynamic) {
		// Arrange:
		auto sender = test::GenerateRandomByteArray<Address>();
		auto recipient = test::GenerateRandomUnresolvedAddress();
		auto notification = model::BalanceTransferNotification(
				sender,
				recipient,
				test::UnresolveXor(Currency_Mosaic_Id),
				Amount(0x8000'0000'0000'0000),
				model::BalanceTransferNotification::AmountType::Dynamic);

		auto cache = test::CreateCache(sender, { { Currency_Mosaic_Id, Amount(234) } });
		SetDynamicFeeMultiplier(cache, BlockFeeMultiplier(2));

		auto cacheView = cache.createView();
		auto readOnlyCache = cacheView.toReadOnly();
		auto context = test::CreateValidatorContext(Height(1), readOnlyCache);

		auto pValidator = CreateBalanceTransferValidator();

		// Sanity:
		EXPECT_EQ(0u, notification.Amount.unwrap() * context.Cache.dependentState().DynamicFeeMultiplier.unwrap());

		// Act:
		auto result = test::ValidateNotification(*pValidator, notification, context);

		// Assert:
		EXPECT_EQ(Failure_Core_Insufficient_Balance, result);
	}

	// endregion

	// region variable balance tests

	VARIABLE_BALANCE_TEST(Currency) {
		// Arrange:
		auto sender = test::GenerateRandomByteArray<Address>();
		auto recipient = test::GenerateRandomUnresolvedAddress();
		auto notification = model::BalanceTransferNotification(sender, recipient, test::UnresolveXor(Currency_Mosaic_Id), Amount(234));
		auto cache = test::CreateCache(sender, { { Currency_Mosaic_Id, TTraits::Adjust(Amount(234)) } });

		// Assert:
		AssertValidationResult<TValidatorTraits>(TTraits::Expected_Result, cache, notification);
	}

	VARIABLE_BALANCE_TEST(OtherMosaic) {
		// Arrange:
		auto sender = test::GenerateRandomByteArray<Address>();
		auto recipient = test::GenerateRandomUnresolvedAddress();
		auto notification = model::BalanceTransferNotification(sender, recipient, test::UnresolveXor(MosaicId(12)), Amount(234));
		auto cache = test::CreateCache(sender, { { MosaicId(12), TTraits::Adjust(Amount(234)) } });

		// Assert:
		AssertValidationResult<TValidatorTraits>(TTraits::Expected_Result, cache, notification);
	}

	// endregion
}}
