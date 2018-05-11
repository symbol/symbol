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
