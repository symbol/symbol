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
#include "tests/test/plugins/AccountObserverTestContext.h"
#include "tests/test/plugins/ObserverTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace observers {

#define TEST_CLASS TransactionFeeActivityObserverTests

	DEFINE_COMMON_OBSERVER_TESTS(TransactionFeeActivity,)

	namespace {
		constexpr auto Notification_Height = Height(100);
		constexpr auto Importance_Height = model::ImportanceHeight(98);

		class TestContext : public test::AccountObserverTestContext {
		public:
			explicit TestContext(NotifyMode notifyMode)
					: test::AccountObserverTestContext(notifyMode, Notification_Height, CreateBlockChainConfiguration())
			{}

		public:
			auto addAccount(const Address& address, Amount totalFeesPaid) {
				auto& accountStateCache = cache().sub<cache::AccountStateCache>();
				accountStateCache.addAccount(address, Height(123));

				auto accountStateIter = accountStateCache.find(address);
				if (Amount(0) != totalFeesPaid) {
					accountStateIter.get().ActivityBuckets.update(Importance_Height, [totalFeesPaid](auto& bucket) {
						bucket.TotalFeesPaid = totalFeesPaid;
					});
				}

				return accountStateIter;
			}

		private:
			static model::BlockChainConfiguration CreateBlockChainConfiguration() {
				auto config = model::BlockChainConfiguration::Uninitialized();
				config.ImportanceGrouping = 2;
				return config;
			}
		};

		void RunTest(NotifyMode notifyMode, Amount initialTotalFeesPaid, Amount fee, Amount expectedTotalFeesPaid) {
			// Arrange:
			TestContext context(notifyMode);
			auto pObserver = CreateTransactionFeeActivityObserver();

			auto sender = test::GenerateRandomByteArray<Address>();
			auto senderAccountStateIter = context.addAccount(sender, initialTotalFeesPaid);

			auto notification = model::TransactionFeeNotification(sender, 0, fee, Amount(222));

			// Act:
			test::ObserveNotification(*pObserver, notification, context);

			// Assert:
			const auto& activityBucket = senderAccountStateIter.get().ActivityBuckets.get(Importance_Height);
			EXPECT_EQ(expectedTotalFeesPaid, activityBucket.TotalFeesPaid);
		}
	}

	TEST(TEST_CLASS, TotalFeesPaidIsNotUpdatedWhenNotificationFeeIsZero_Commit) {
		RunTest(NotifyMode::Commit, Amount(333), Amount(0), Amount(333));
	}

	TEST(TEST_CLASS, TotalFeesPaidIsNotUpdatedWhenNotificationFeeIsZero_Rollback) {
		RunTest(NotifyMode::Rollback, Amount(333), Amount(0), Amount(333));
	}

	TEST(TEST_CLASS, TotalFeesPaidIsUpdatedWhenNotificationFeeIsNonzero_Commit) {
		RunTest(NotifyMode::Commit, Amount(333), Amount(100), Amount(433));
	}

	TEST(TEST_CLASS, TotalFeesPaidIsUpdatedWhenNotificationFeeIsNonzero_Rollback) {
		RunTest(NotifyMode::Rollback, Amount(333), Amount(100), Amount(233));
	}

	TEST(TEST_CLASS, ZeroFeesPaidDoesNotCreateBucket_Commit) {
		// Arrange:
		TestContext context(NotifyMode::Commit);
		auto pObserver = CreateTransactionFeeActivityObserver();

		auto sender = test::GenerateRandomByteArray<Address>();
		auto senderAccountStateIter = context.addAccount(sender, Amount(0));

		auto notification = model::TransactionFeeNotification(sender, 0, Amount(0), Amount(222));

		// Act:
		test::ObserveNotification(*pObserver, notification, context);

		// Assert: no bucket was created
		const auto& activityBucket = senderAccountStateIter.get().ActivityBuckets.get(Importance_Height);
		EXPECT_EQ(model::ImportanceHeight(), activityBucket.StartHeight);
	}
}}
