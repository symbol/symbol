/**
*** Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
*** Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
*** All rights reserved.
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
#include "tests/test/core/NotificationTestUtils.h"
#include "tests/test/plugins/ObserverTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace observers {

#define TEST_CLASS TotalTransactionsObserverTests

	DEFINE_COMMON_OBSERVER_TESTS(TotalTransactions,)

	namespace {
		constexpr size_t Current_Height = 10;

		model::BlockNotification CreateBlockNotification(uint32_t numTransactions) {
			auto notification = test::CreateBlockNotification();
			notification.NumTransactions = numTransactions;
			return notification;
		}
	}

	TEST(TEST_CLASS, ObserverIncrementsTotalNumberOfTransactionsInModeCommit) {
		// Arrange:
		test::ObserverTestContext context(NotifyMode::Commit, Height(Current_Height + 1));
		context.state().NumTotalTransactions = 123;
		auto pObserver = CreateTotalTransactionsObserver();
		auto notification = CreateBlockNotification(1123);

		// Act:
		ObserveNotification(*pObserver, notification, context);

		// Assert:
		EXPECT_EQ(1246u, context.state().NumTotalTransactions);
	}

	TEST(TEST_CLASS, ObserverDecrementsTotalNumberOfTransactionsInModeRollback) {
		// Arrange:
		test::ObserverTestContext context(NotifyMode::Rollback, Height(Current_Height + 1));
		context.state().NumTotalTransactions = 1246;
		auto pObserver = CreateTotalTransactionsObserver();
		auto notification = CreateBlockNotification(1123);

		// Act:
		ObserveNotification(*pObserver, notification, context);

		// Assert:
		EXPECT_EQ(123u, context.state().NumTotalTransactions);
	}
}}
