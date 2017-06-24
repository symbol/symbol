#include "src/observers/Observers.h"
#include "tests/test/cache/BalanceTransferTestUtils.h"
#include "tests/test/plugins/AccountObserverTestContext.h"
#include "tests/test/plugins/ObserverTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace observers {

	DEFINE_COMMON_OBSERVER_TESTS(NemesisBalanceChange,)

#define TEST_CLASS NemesisBalanceChangeObserverTests

	namespace {
		template<typename TTraits>
		void AssertCommitObservation() {
			// Arrange:
			test::AccountObserverTestContext context(NotifyMode::Commit, Height(1));
			auto pObserver = CreateNemesisBalanceChangeObserver();

			auto sender = test::GenerateRandomData<Key_Size>();
			auto recipient = test::GenerateRandomData<Address_Decoded_Size>();
			auto notification = TTraits::CreateNotification(sender, recipient);

			test::SetCacheBalances(context.cache(), sender, TTraits::GetInitialSenderBalances());

			// Act:
			test::ObserveNotification(*pObserver, notification, context);

			// Assert:
			test::AssertBalances(context.cache(), sender, TTraits::GetFinalSenderBalances());
		}
	}

#define DEFINE_BALANCE_OBSERVATION_TESTS(TEST_NAME) \
	TEST(TEST_CLASS, TEST_NAME##_Commit) { AssertCommitObservation<TEST_NAME##Traits>(); }

	// region no-op

	namespace {
		void AssertNoOp(NotifyMode notifyMode, Height height) {
			// Arrange:
			CATAPULT_LOG(debug) << "mode " << notifyMode << ", height " << height;
			test::AccountObserverTestContext context(notifyMode, height);
			auto pObserver = CreateNemesisBalanceChangeObserver();

			auto sender = test::GenerateRandomData<Key_Size>();
			auto recipient = test::GenerateRandomData<Address_Decoded_Size>();
			auto notification = model::BalanceTransferNotification(sender, recipient, Xem_Id, Amount(123));

			context.setAccountBalance(sender, 1000);

			// Act:
			test::ObserveNotification(*pObserver, notification, context);

			// Assert:
			test::AssertBalances(context.cache(), sender, { { Xem_Id, Amount(1000) } });
		}
	}

	TEST(TEST_CLASS, BalanceObserverIsSkippedWhenHeightIsGreaterThanOne) {
		// Assert:
		AssertNoOp(NotifyMode::Commit, Height(2));
		AssertNoOp(NotifyMode::Commit, Height(999));
	}

	TEST(TEST_CLASS, BalanceObserverIsSkippedWhenModeIsRollback) {
		// Assert:
		AssertNoOp(NotifyMode::Rollback, Height(1));
		AssertNoOp(NotifyMode::Rollback, Height(2));
		AssertNoOp(NotifyMode::Rollback, Height(999));
	}

	// endregion

	// region xem

	namespace {
		struct XemTraits {
			static auto CreateNotification(const Key& sender, const Address& recipient) {
				return model::BalanceTransferNotification(sender, recipient, Xem_Id, Amount(234));
			}

			static test::BalanceTransfers GetInitialSenderBalances() {
				return { { Xem_Id, Amount(1000) } };
			}

			static test::BalanceTransfers GetFinalSenderBalances() {
				return { { Xem_Id, Amount(1000 - 234) } };
			}
		};
	}

	DEFINE_BALANCE_OBSERVATION_TESTS(Xem)

	// endregion

	// region other mosaic

	namespace {
		struct OtherMosaicTraits {
			static auto CreateNotification(const Key& sender, const Address& recipient) {
				return model::BalanceTransferNotification(sender, recipient, MosaicId(12), Amount(234));
			}

			static test::BalanceTransfers GetInitialSenderBalances() {
				return { { Xem_Id, Amount(1000) }, { MosaicId(12), Amount(1200) } };
			}

			static test::BalanceTransfers GetFinalSenderBalances() {
				return { { Xem_Id, Amount(1000) }, { MosaicId(12), Amount(1200 - 234) } };
			}
		};
	}

	DEFINE_BALANCE_OBSERVATION_TESTS(OtherMosaic)

	// endregion
}}
