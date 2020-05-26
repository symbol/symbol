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

#define TEST_CLASS BalanceTransferObserverTests

	DEFINE_COMMON_OBSERVER_TESTS(BalanceTransfer,)

	namespace {
		constexpr auto Dynamic_Fee_Multiplier = BlockFeeMultiplier(117);

		template<typename TTraits>
		void AssertCommitObservation() {
			// Arrange:
			test::AccountObserverTestContext context(NotifyMode::Commit);
			context.state().DynamicFeeMultiplier = Dynamic_Fee_Multiplier;
			auto pObserver = CreateBalanceTransferObserver();

			auto sender = test::GenerateRandomByteArray<Address>();
			auto recipient = test::GenerateRandomByteArray<Address>();
			auto notification = TTraits::CreateNotification(sender, test::UnresolveXor(recipient));

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
			context.state().DynamicFeeMultiplier = Dynamic_Fee_Multiplier;
			auto pObserver = CreateBalanceTransferObserver();

			auto sender = test::GenerateRandomByteArray<Address>();
			auto recipient = test::GenerateRandomByteArray<Address>();
			auto notification = TTraits::CreateNotification(sender, test::UnresolveXor(recipient));

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
	TEST(TEST_CLASS, CanTransfer##TEST_NAME##_Commit) { AssertCommitObservation<TEST_NAME##Traits>(); } \
	TEST(TEST_CLASS, CanTransfer##TEST_NAME##_Rollback) { AssertRollbackObservation<TEST_NAME##Traits>(); }

	// region single mosaic

	namespace {
		constexpr auto Currency_Mosaic_Id = MosaicId(1234);

		struct SingleMosaicStaticTraits {
			static auto CreateNotification(const Address& sender, const UnresolvedAddress& recipient) {
				return model::BalanceTransferNotification(sender, recipient, test::UnresolveXor(Currency_Mosaic_Id), Amount(234));
			}

			static test::BalanceTransfers GetInitialSenderBalances() {
				return { { Currency_Mosaic_Id, Amount(1000) } };
			}

			static test::BalanceTransfers GetFinalSenderBalances() {
				return { { Currency_Mosaic_Id, Amount(1000 - 234) } };
			}

			static test::BalanceTransfers GetInitialRecipientBalances() {
				return { { Currency_Mosaic_Id, Amount(750) } };
			}

			static test::BalanceTransfers GetFinalRecipientBalances() {
				return { { Currency_Mosaic_Id, Amount(750 + 234) } };
			}
		};

		struct SingleMosaicDynamicTraits : public SingleMosaicStaticTraits {
			static auto CreateNotification(const Address& sender, const UnresolvedAddress& recipient) {
				return model::BalanceTransferNotification(
						sender,
						recipient,
						test::UnresolveXor(Currency_Mosaic_Id),
						Amount(234 / Dynamic_Fee_Multiplier.unwrap()),
						model::BalanceTransferNotification::AmountType::Dynamic);
			}
		};
	}

	DEFINE_BALANCE_OBSERVATION_TESTS(SingleMosaicStatic)
	DEFINE_BALANCE_OBSERVATION_TESTS(SingleMosaicDynamic)

	// endregion

	// region multiple mosaics

	namespace {
		struct MultipleMosaicStaticTraits {
			static auto CreateNotification(const Address& sender, const UnresolvedAddress& recipient) {
				return model::BalanceTransferNotification(sender, recipient, test::UnresolveXor(MosaicId(12)), Amount(234));
			}

			static test::BalanceTransfers GetInitialSenderBalances() {
				return { { Currency_Mosaic_Id, Amount(1000) }, { MosaicId(12), Amount(1200) } };
			}

			static test::BalanceTransfers GetFinalSenderBalances() {
				return { { Currency_Mosaic_Id, Amount(1000) }, { MosaicId(12), Amount(1200 - 234) } };
			}

			static test::BalanceTransfers GetInitialRecipientBalances() {
				return { { Currency_Mosaic_Id, Amount(750) }, { MosaicId(12), Amount(500) } };
			}

			static test::BalanceTransfers GetFinalRecipientBalances() {
				return { { Currency_Mosaic_Id, Amount(750) }, { MosaicId(12), Amount(500 + 234) } };
			}
		};
	}

	struct MultipleMosaicDynamicTraits : public MultipleMosaicStaticTraits {
		static auto CreateNotification(const Address& sender, const UnresolvedAddress& recipient) {
			return model::BalanceTransferNotification(
					sender,
					recipient,
					test::UnresolveXor(MosaicId(12)),
					Amount(234 / Dynamic_Fee_Multiplier.unwrap()),
					model::BalanceTransferNotification::AmountType::Dynamic);
		}
	};

	DEFINE_BALANCE_OBSERVATION_TESTS(MultipleMosaicStatic)
	DEFINE_BALANCE_OBSERVATION_TESTS(MultipleMosaicDynamic)

	// endregion
}}
