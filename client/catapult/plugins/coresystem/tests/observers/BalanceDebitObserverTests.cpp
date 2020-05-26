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
#include "tests/test/plugins/AccountObserverTestContext.h"
#include "tests/test/plugins/ObserverTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace observers {

#define TEST_CLASS BalanceDebitObserverTests

	DEFINE_COMMON_OBSERVER_TESTS(BalanceDebit,)

	namespace {
		template<typename TTraits>
		void AssertCommitObservation() {
			// Arrange:
			test::AccountObserverTestContext context(NotifyMode::Commit);
			auto pObserver = CreateBalanceDebitObserver();

			auto sender = test::GenerateRandomByteArray<Address>();
			auto notification = TTraits::CreateNotification(sender);

			test::SetCacheBalances(context.cache(), sender, TTraits::GetInitialSenderBalances());

			// Act:
			test::ObserveNotification(*pObserver, notification, context);

			// Assert:
			test::AssertBalances(context.cache(), sender, TTraits::GetFinalSenderBalances());
		}

		template<typename TTraits>
		void AssertRollbackObservation() {
			// Arrange:
			test::AccountObserverTestContext context(NotifyMode::Rollback);
			auto pObserver = CreateBalanceDebitObserver();

			auto sender = test::GenerateRandomByteArray<Address>();
			auto notification = TTraits::CreateNotification(sender);

			test::SetCacheBalances(context.cache(), sender, TTraits::GetFinalSenderBalances());

			// Act:
			test::ObserveNotification(*pObserver, notification, context);

			// Assert:
			test::AssertBalances(context.cache(), sender, TTraits::GetInitialSenderBalances());
		}
	}

#define DEFINE_BALANCE_OBSERVATION_TESTS(TEST_NAME) \
	TEST(TEST_CLASS, CanTransfer##TEST_NAME##_Commit) { AssertCommitObservation<TEST_NAME##Traits>(); } \
	TEST(TEST_CLASS, CanTransfer##TEST_NAME##_Rollback) { AssertRollbackObservation<TEST_NAME##Traits>(); }

	// region single mosaic

	namespace {
		constexpr auto Currency_Mosaic_Id = MosaicId(1234);

		struct SingleMosaicTraits {
			static auto CreateNotification(const Address& sender) {
				return model::BalanceDebitNotification(sender, test::UnresolveXor(Currency_Mosaic_Id), Amount(234));
			}

			static test::BalanceTransfers GetInitialSenderBalances() {
				return { { Currency_Mosaic_Id, Amount(1000) } };
			}

			static test::BalanceTransfers GetFinalSenderBalances() {
				return { { Currency_Mosaic_Id, Amount(1000 - 234) } };
			}
		};
	}

	DEFINE_BALANCE_OBSERVATION_TESTS(SingleMosaic)

	// endregion

	// region multiple mosaics

	namespace {
		struct MultipleMosaicTraits {
			static auto CreateNotification(const Address& sender) {
				return model::BalanceDebitNotification(sender, test::UnresolveXor(MosaicId(12)), Amount(234));
			}

			static test::BalanceTransfers GetInitialSenderBalances() {
				return { { Currency_Mosaic_Id, Amount(1000) }, { MosaicId(12), Amount(1200) } };
			}

			static test::BalanceTransfers GetFinalSenderBalances() {
				return { { Currency_Mosaic_Id, Amount(1000) }, { MosaicId(12), Amount(1200 - 234) } };
			}
		};
	}

	DEFINE_BALANCE_OBSERVATION_TESTS(MultipleMosaic)

	// endregion
}}
