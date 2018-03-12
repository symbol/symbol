#include "src/observers/Observers.h"
#include "tests/test/cache/BalanceTransferTestUtils.h"
#include "tests/test/core/NotificationTestUtils.h"
#include "tests/test/plugins/AccountObserverTestContext.h"
#include "tests/test/plugins/ObserverTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace observers {

#define TEST_CLASS HarvestFeeObserverTests

	DEFINE_COMMON_OBSERVER_TESTS(HarvestFee,)

	namespace {
		template<typename TAction>
		void RunHarvestFeeObserverTest(NotifyMode notifyMode, TAction action) {
			// Arrange:
			test::AccountObserverTestContext context(notifyMode);

			auto pObserver = CreateHarvestFeeObserver();

			// Act + Assert:
			action(context, *pObserver);
		}
	}

	TEST(TEST_CLASS, CommitCreditsHarvester) {
		// Arrange:
		RunHarvestFeeObserverTest(NotifyMode::Commit, [](test::AccountObserverTestContext& context, const auto& observer) {
			auto signer = test::GenerateRandomData<Key_Size>();
			test::SetCacheBalances(context.cache(), signer, { { Xem_Id, Amount(987) } });

			auto notification = test::CreateBlockNotification(signer);
			notification.TotalFee = Amount(123);

			// Act:
			test::ObserveNotification(observer, notification, context);

			// Assert:
			test::AssertBalances(context.cache(), signer, { { Xem_Id, Amount(987 + 123) } });
		});
	}

	TEST(TEST_CLASS, RollbackDebitsHarvester) {
		// Arrange:
		RunHarvestFeeObserverTest(NotifyMode::Rollback, [](test::AccountObserverTestContext& context, const auto& observer) {
			auto signer = test::GenerateRandomData<Key_Size>();
			test::SetCacheBalances(context.cache(), signer, { { Xem_Id, Amount(987 + 123) } });

			auto notification = test::CreateBlockNotification(signer);
			notification.TotalFee = Amount(123);

			// Act:
			test::ObserveNotification(observer, notification, context);

			// Assert:
			test::AssertBalances(context.cache(), signer, { { Xem_Id, Amount(987) } });
		});
	}
}}
