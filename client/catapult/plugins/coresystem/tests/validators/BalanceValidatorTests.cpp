#include "src/validators/Validators.h"
#include "catapult/cache/CatapultCache.h"
#include "catapult/cache_core/AccountStateCache.h"
#include "catapult/model/Address.h"
#include "catapult/constants.h"
#include "tests/test/cache/BalanceTransferTestUtils.h"
#include "tests/test/cache/CacheTestUtils.h"
#include "tests/test/plugins/ValidatorTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace validators {

	DEFINE_COMMON_VALIDATOR_TESTS(BalanceTransfer,)
	DEFINE_COMMON_VALIDATOR_TESTS(BalanceReserve,)

#define TEST_CLASS BalanceValidatorTests // used to generate unique function names in macros
#define TRANSFER_TEST_CLASS BalanceTransferValidatorTests
#define RESERVE_TEST_CLASS BalanceReserveValidatorTests

	namespace {
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
			// Arrange:
			auto cacheView = cache.createView();
			auto readOnlyCache = cacheView.toReadOnly();
			auto context = test::CreateValidatorContext(Height(1), readOnlyCache);

			// Act:
			auto result = TTraits::Validate(notification, context);

			// Assert:
			EXPECT_EQ(expectedResult, result);
		}

		struct TransferTraits {
			static ValidationResult Validate(
					const model::BalanceTransferNotification& notification,
					const validators::ValidatorContext& context) {
				auto pValidator = CreateBalanceTransferValidator();
				return test::ValidateNotification(*pValidator, notification, context);
			}
		};

		struct ReserveTraits {
			static ValidationResult Validate(
					const model::BalanceTransferNotification& notification,
					const validators::ValidatorContext& context) {
				auto pValidator = CreateBalanceReserveValidator();

				// - map transfer notification to a reserve notification
				auto reserveNotification = model::BalanceReserveNotification(
						notification.Sender,
						notification.MosaicId,
						notification.Amount);

				return test::ValidateNotification(*pValidator, reserveNotification, context);
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
		auto sender = test::GenerateRandomData<Key_Size>();
		auto recipient = test::GenerateRandomData<Address_Decoded_Size>();
		auto notification = model::BalanceTransferNotification(sender, recipient, Xem_Id, Amount(0));
		auto cache = test::CreateEmptyCatapultCache();

		// Assert:
		AssertValidationResult<TValidatorTraits>(Failure_Core_Insufficient_Balance, cache, notification);
	}

	VARIABLE_BALANCE_TEST(Xem) {
		// Arrange:
		auto sender = test::GenerateRandomData<Key_Size>();
		auto recipient = test::GenerateRandomData<Address_Decoded_Size>();
		auto notification = model::BalanceTransferNotification(sender, recipient, Xem_Id, Amount(234));
		auto cache = test::CreateCache(sender, { { Xem_Id, TTraits::Adjust(Amount(234)) } });

		// Assert:
		AssertValidationResult<TValidatorTraits>(TTraits::Expected_Result, cache, notification);
	}

	VARIABLE_BALANCE_TEST(Xem_SeededByAddress) {
		// Arrange:
		auto sender = test::GenerateRandomData<Key_Size>();
		auto recipient = test::GenerateRandomData<Address_Decoded_Size>();
		auto notification = model::BalanceTransferNotification(sender, recipient, Xem_Id, Amount(234));

		// - seed the sender by address
		auto cache = test::CreateEmptyCatapultCache();
		auto senderAddress = model::PublicKeyToAddress(sender, cache.sub<cache::AccountStateCache>().networkIdentifier());
		{
			auto delta = cache.createDelta();
			test::SetCacheBalances(delta, senderAddress, { { Xem_Id, TTraits::Adjust(Amount(234)) } });
			cache.commit(Height());
		}

		// Assert:
		AssertValidationResult<TValidatorTraits>(TTraits::Expected_Result, cache, notification);
	}

	VARIABLE_BALANCE_TEST(OtherMosaic) {
		// Arrange:
		auto sender = test::GenerateRandomData<Key_Size>();
		auto recipient = test::GenerateRandomData<Address_Decoded_Size>();
		auto notification = model::BalanceTransferNotification(sender, recipient, MosaicId(12), Amount(234));
		auto cache = test::CreateCache(sender, { { MosaicId(12), TTraits::Adjust(Amount(234)) } });

		// Assert:
		AssertValidationResult<TValidatorTraits>(TTraits::Expected_Result, cache, notification);
	}
}}
