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

	DEFINE_COMMON_OBSERVER_TESTS(Balance,)

	namespace {
		template<typename TTraits>
		void AssertCommitObservation() {
			// Arrange:
			test::AccountObserverTestContext context(NotifyMode::Commit);
			auto pObserver = CreateBalanceObserver();

			auto sender = test::GenerateRandomData<Key_Size>();
			auto recipient = test::GenerateRandomData<Address_Decoded_Size>();
			auto notification = TTraits::CreateNotification(sender, recipient);

			test::SetCacheBalances(context.cache(), sender, TTraits::GetInitialSenderBalances());
			test::SetCacheBalances(context.cache(), recipient, TTraits::GetInitialRecipientBalances());

			// Act:
			test::ObserveNotification(*pObserver, notification, context);

			// Assert:
			test::AssertBalances(context.cache(), sender, TTraits::GetFinalSenderBalances());
			test::AssertBalances(context.cache(), recipient, TTraits::GetFinalRecipientBalances());
		}

		template<typename TTraits>
		void AssertRollbackObservation() {
			// Arrange:
			test::AccountObserverTestContext context(NotifyMode::Rollback);
			auto pObserver = CreateBalanceObserver();

			auto sender = test::GenerateRandomData<Key_Size>();
			auto recipient = test::GenerateRandomData<Address_Decoded_Size>();
			auto notification = TTraits::CreateNotification(sender, recipient);

			test::SetCacheBalances(context.cache(), sender, TTraits::GetFinalSenderBalances());
			test::SetCacheBalances(context.cache(), recipient, TTraits::GetFinalRecipientBalances());

			// Act:
			test::ObserveNotification(*pObserver, notification, context);

			// Assert:
			test::AssertBalances(context.cache(), sender, TTraits::GetInitialSenderBalances());
			test::AssertBalances(context.cache(), recipient, TTraits::GetInitialRecipientBalances());
		}
	}

#define DEFINE_BALANCE_OBSERVATION_TESTS(TEST_NAME) \
	TEST(BalanceObserverTests, CanTransfer##TEST_NAME##_Commit) { AssertCommitObservation<TEST_NAME##Traits>(); } \
	TEST(BalanceObserverTests, CanTransfer##TEST_NAME##_Rollback) { AssertRollbackObservation<TEST_NAME##Traits>(); }

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

			static test::BalanceTransfers GetInitialRecipientBalances() {
				return { { Xem_Id, Amount(750) } };
			}

			static test::BalanceTransfers GetFinalRecipientBalances() {
				return { { Xem_Id, Amount(750 + 234) } };
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

			static test::BalanceTransfers GetInitialRecipientBalances() {
				return { { Xem_Id, Amount(750) }, { MosaicId(12), Amount(500) } };
			}

			static test::BalanceTransfers GetFinalRecipientBalances() {
				return { { Xem_Id, Amount(750) }, { MosaicId(12), Amount(500 + 234) } };
			}
		};
	}

	DEFINE_BALANCE_OBSERVATION_TESTS(OtherMosaic)

	// endregion
}}
