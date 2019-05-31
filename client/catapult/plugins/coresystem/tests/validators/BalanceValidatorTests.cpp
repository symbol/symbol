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
#include "catapult/model/Address.h"
#include "tests/test/cache/BalanceTransferTestUtils.h"
#include "tests/test/cache/CacheTestUtils.h"
#include "tests/test/core/AddressTestUtils.h"
#include "tests/test/plugins/ValidatorTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace validators {

	DEFINE_COMMON_VALIDATOR_TESTS(BalanceTransfer,)
	DEFINE_COMMON_VALIDATOR_TESTS(BalanceDebit,)

#define TEST_CLASS BalanceValidatorTests // used to generate unique function names in macros
#define TRANSFER_TEST_CLASS TransferBalanceValidatorTests
#define RESERVE_TEST_CLASS DebitBalanceValidatorTests

	namespace {
		constexpr auto Currency_Mosaic_Id = MosaicId(1234);

		// region traits

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

		template<typename TTraits>
		void AssertValidationResult(
				ValidationResult expectedResult,
				const cache::CatapultCache& cache,
				const model::BalanceTransferNotification& notification) {
			// Act:
			auto result = TTraits::Validate(notification, cache);

			// Assert:
			EXPECT_EQ(expectedResult, result);
		}

		struct TransferTraits {
			static ValidationResult Validate(const model::BalanceTransferNotification& notification, const cache::CatapultCache& cache) {
				auto pValidator = CreateBalanceTransferValidator();
				return test::ValidateNotification(*pValidator, notification, cache);
			}
		};

		struct ReserveTraits {
			static ValidationResult Validate(const model::BalanceTransferNotification& notification, const cache::CatapultCache& cache) {
				auto pValidator = CreateBalanceDebitValidator();

				// - map transfer notification to a reserve notification
				auto reserveNotification = model::BalanceDebitNotification(
						notification.Sender,
						notification.MosaicId,
						notification.Amount);

				return test::ValidateNotification(*pValidator, reserveNotification, cache);
			}
		};

#define TRANSFER_OR_RESERVE_TEST(TEST_NAME) \
	template<typename TValidatorTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TRANSFER_TEST_CLASS, TEST_NAME) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<TransferTraits>(); } \
	TEST(RESERVE_TEST_CLASS, TEST_NAME) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<ReserveTraits>(); } \
	template<typename TValidatorTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

#define VARIABLE_BALANCE_TEST(TEST_NAME) \
	template<typename TValidatorTraits, typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TRANSFER_TEST_CLASS, SuccessWhenBalanceIsGreater_##TEST_NAME) { \
		TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<TransferTraits, BalanceGreaterTraits>(); \
	} \
	TEST(TRANSFER_TEST_CLASS, SuccessWhenBalanceIsExact_##TEST_NAME) { \
		TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<TransferTraits, BalanceEqualTraits>(); \
	} \
	TEST(TRANSFER_TEST_CLASS, FailureWhenBalanceIsTooSmall_##TEST_NAME) { \
		TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<TransferTraits, BalanceLessTraits>(); \
	} \
	TEST(RESERVE_TEST_CLASS, SuccessWhenBalanceIsGreater_##TEST_NAME) { \
		TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<ReserveTraits, BalanceGreaterTraits>(); \
	} \
	TEST(RESERVE_TEST_CLASS, SuccessWhenBalanceIsExact_##TEST_NAME) { \
		TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<ReserveTraits, BalanceEqualTraits>(); \
	} \
	TEST(RESERVE_TEST_CLASS, FailureWhenBalanceIsTooSmall_##TEST_NAME) { \
		TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<ReserveTraits, BalanceLessTraits>(); \
	} \
	template<typename TValidatorTraits, typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

		// endregion
	}

	TRANSFER_OR_RESERVE_TEST(FailureWhenTransactionSignerIsUnknown) {
		// Arrange: do not register the signer with the cache
		auto sender = test::GenerateRandomByteArray<Key>();
		auto recipient = test::GenerateRandomUnresolvedAddress();
		auto notification = model::BalanceTransferNotification(sender, recipient, test::UnresolveXor(Currency_Mosaic_Id), Amount(0));
		auto cache = test::CreateEmptyCatapultCache();

		// Assert:
		AssertValidationResult<TValidatorTraits>(Failure_Core_Insufficient_Balance, cache, notification);
	}

	VARIABLE_BALANCE_TEST(Currency) {
		// Arrange:
		auto sender = test::GenerateRandomByteArray<Key>();
		auto recipient = test::GenerateRandomUnresolvedAddress();
		auto notification = model::BalanceTransferNotification(sender, recipient, test::UnresolveXor(Currency_Mosaic_Id), Amount(234));
		auto cache = test::CreateCache(sender, { { Currency_Mosaic_Id, TTraits::Adjust(Amount(234)) } });

		// Assert:
		AssertValidationResult<TValidatorTraits>(TTraits::Expected_Result, cache, notification);
	}

	VARIABLE_BALANCE_TEST(Currency_SeededByAddress) {
		// Arrange:
		auto sender = test::GenerateRandomByteArray<Key>();
		auto recipient = test::GenerateRandomUnresolvedAddress();
		auto notification = model::BalanceTransferNotification(sender, recipient, test::UnresolveXor(Currency_Mosaic_Id), Amount(234));

		// - seed the sender by address
		auto cache = test::CreateEmptyCatapultCache();
		auto senderAddress = model::PublicKeyToAddress(sender, cache.sub<cache::AccountStateCache>().networkIdentifier());
		{
			auto delta = cache.createDelta();
			test::SetCacheBalances(delta, senderAddress, { { Currency_Mosaic_Id, TTraits::Adjust(Amount(234)) } });
			cache.commit(Height());
		}

		// Assert:
		AssertValidationResult<TValidatorTraits>(TTraits::Expected_Result, cache, notification);
	}

	VARIABLE_BALANCE_TEST(OtherMosaic) {
		// Arrange:
		auto sender = test::GenerateRandomByteArray<Key>();
		auto recipient = test::GenerateRandomUnresolvedAddress();
		auto notification = model::BalanceTransferNotification(sender, recipient, test::UnresolveXor(MosaicId(12)), Amount(234));
		auto cache = test::CreateCache(sender, { { MosaicId(12), TTraits::Adjust(Amount(234)) } });

		// Assert:
		AssertValidationResult<TValidatorTraits>(TTraits::Expected_Result, cache, notification);
	}
}}
