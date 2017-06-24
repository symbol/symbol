#include "src/validators/Validators.h"
#include "catapult/cache/AccountStateCache.h"
#include "catapult/cache/CatapultCache.h"
#include "catapult/model/Address.h"
#include "catapult/constants.h"
#include "tests/test/cache/BalanceTransferTestUtils.h"
#include "tests/test/cache/CacheTestUtils.h"
#include "tests/test/plugins/ValidatorTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace validators {

	DEFINE_COMMON_VALIDATOR_TESTS(Balance,)

#define TEST_CLASS BalanceValidatorTests

	namespace {
		void AssertValidationResult(
				const cache::CatapultCache& cache,
				const model::BalanceTransferNotification& notification,
				ValidationResult expectedResult) {
			// Arrange:
			auto pValidator = CreateBalanceValidator();
			auto cacheView = cache.createView();
			auto readOnlyCache = cacheView.toReadOnly();
			auto context = test::CreateValidatorContext(Height(1), readOnlyCache);

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification, context);

			// Assert:
			EXPECT_EQ(expectedResult, result);
		}

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

#define VARIABLE_BALANCE_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, SuccessWhenBalanceIsGreater_##TEST_NAME) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<BalanceGreaterTraits>(); } \
	TEST(TEST_CLASS, SuccessWhenBalanceIsExact_##TEST_NAME) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<BalanceEqualTraits>(); } \
	TEST(TEST_CLASS, FailureWhenBalanceIsTooSmall_##TEST_NAME) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<BalanceLessTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

		// endregion
	}

	TEST(TEST_CLASS, FailureWhenTransactionSignerIsUnknown) {
		// Arrange: do not register the signer with the cache
		auto sender = test::GenerateRandomData<Key_Size>();
		auto recipient = test::GenerateRandomData<Address_Decoded_Size>();
		auto notification = model::BalanceTransferNotification(sender, recipient, Xem_Id, Amount(0));
		auto cache = test::CreateEmptyCatapultCache();

		// Assert:
		AssertValidationResult(cache, notification, Failure_Core_Insufficient_Balance);
	}

	VARIABLE_BALANCE_TEST(Xem) {
		// Arrange:
		auto sender = test::GenerateRandomData<Key_Size>();
		auto recipient = test::GenerateRandomData<Address_Decoded_Size>();
		auto notification = model::BalanceTransferNotification(sender, recipient, Xem_Id, Amount(234));
		auto cache = test::CreateCache(sender, { { Xem_Id, TTraits::Adjust(Amount(234)) } });

		// Assert:
		AssertValidationResult(cache, notification, TTraits::Expected_Result);
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
		AssertValidationResult(cache, notification, TTraits::Expected_Result);
	}

	VARIABLE_BALANCE_TEST(OtherMosaic) {
		// Arrange:
		auto sender = test::GenerateRandomData<Key_Size>();
		auto recipient = test::GenerateRandomData<Address_Decoded_Size>();
		auto notification = model::BalanceTransferNotification(sender, recipient, MosaicId(12), Amount(234));
		auto cache = test::CreateCache(sender, { { MosaicId(12), TTraits::Adjust(Amount(234)) } });

		// Assert:
		AssertValidationResult(cache, notification, TTraits::Expected_Result);
	}
}}
