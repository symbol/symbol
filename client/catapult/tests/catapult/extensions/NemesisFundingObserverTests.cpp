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

#include "catapult/extensions/NemesisFundingObserver.h"
#include "tests/test/core/AddressTestUtils.h"
#include "tests/test/plugins/AccountObserverTestContext.h"
#include "tests/test/plugins/ObserverTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace extensions {

#define TEST_CLASS NemesisFundingObserverTests

	namespace {
		auto CreateBalanceTransferNotification(const Address& sender, UnresolvedMosaicId mosaicId, Amount amount) {
			return model::BalanceTransferNotification(sender, test::GenerateRandomUnresolvedAddress(), mosaicId, amount);
		}
	}

	// region unsupported

	namespace {
		void AssertUnsupported(observers::NotifyMode notifyMode, Height height) {
			// Arrange:
			test::AccountObserverTestContext context(notifyMode, height);
			auto sender = test::GenerateRandomByteArray<Address>();

			NemesisFundingState fundingState;
			auto pObserver = CreateNemesisFundingObserver(sender, fundingState);
			auto notification = CreateBalanceTransferNotification(sender, UnresolvedMosaicId(9876), Amount(12));

			// Act + Assert:
			EXPECT_THROW(test::ObserveNotification(*pObserver, notification, context.observerContext()), catapult_invalid_argument);
		}
	}

	TEST(TEST_CLASS, ObserverDoesNotSupportRollback) {
		AssertUnsupported(observers::NotifyMode::Rollback, Height(1));
	}

	TEST(TEST_CLASS, ObserverDoesNotSupportNonNemesisBlock) {
		AssertUnsupported(observers::NotifyMode::Commit, Height(2));
	}

	TEST(TEST_CLASS, ObserverFailsWhenTransferIsFromNonNemesisAccount) {
		// Arrange:
		test::AccountObserverTestContext context(observers::NotifyMode::Commit, Height(1));
		auto nemesis = test::GenerateRandomByteArray<Address>();
		auto sender = test::GenerateRandomByteArray<Address>();

		NemesisFundingState fundingState;
		auto pObserver = CreateNemesisFundingObserver(nemesis, fundingState);
		auto notification = CreateBalanceTransferNotification(sender, UnresolvedMosaicId(9876), Amount(12));

		// Act + Assert:
		EXPECT_THROW(test::ObserveNotification(*pObserver, notification, context.observerContext()), catapult_invalid_argument);
	}

	// endregion

	// region explicitly funded account

	TEST(TEST_CLASS, ObserverCanProcessExplicitlyFundedNemesisAccount) {
		// Arrange:
		auto mosaicId1 = MosaicId(1234);
		auto mosaicId2 = MosaicId(8844);

		// - pre-fund account
		test::AccountObserverTestContext context(observers::NotifyMode::Commit, Height(1));
		auto sender = test::GenerateRandomByteArray<Address>();

		auto& accountStateCache = context.cache().sub<cache::AccountStateCache>();
		accountStateCache.addAccount(sender, Height(1));
		accountStateCache.find(sender).get().Balances.credit(mosaicId1, Amount(46));
		accountStateCache.find(sender).get().Balances.credit(mosaicId2, Amount(99));

		NemesisFundingState fundingState;
		auto pObserver = CreateNemesisFundingObserver(sender, fundingState);
		auto notification1 = CreateBalanceTransferNotification(sender, test::UnresolveXor(mosaicId1), Amount(12));
		auto notification2 = CreateBalanceTransferNotification(sender, test::UnresolveXor(mosaicId1), Amount(34));
		auto notification3 = CreateBalanceTransferNotification(sender, test::UnresolveXor(mosaicId2), Amount(39));
		auto notification4 = CreateBalanceTransferNotification(sender, test::UnresolveXor(mosaicId2), Amount(60));

		// Act:
		test::ObserveNotification(*pObserver, notification1, context.observerContext()); // NemesisFundingType::Unknown
		test::ObserveNotification(*pObserver, notification2, context.observerContext()); // NemesisFundingType::Explicit
		test::ObserveNotification(*pObserver, notification3, context.observerContext()); // NemesisFundingType::Explicit
		test::ObserveNotification(*pObserver, notification4, context.observerContext()); // NemesisFundingType::Explicit

		// Assert:
		EXPECT_EQ(NemesisFundingType::Explicit, fundingState.FundingType);
		EXPECT_EQ(2u, fundingState.TotalFundedMosaics.size());
		EXPECT_EQ(Amount(46), fundingState.TotalFundedMosaics.get(mosaicId1));
		EXPECT_EQ(Amount(99), fundingState.TotalFundedMosaics.get(mosaicId2));

		auto accountStateIter = accountStateCache.find(sender);
		const auto& balances = accountStateIter.get().Balances;
		EXPECT_EQ(1u, accountStateCache.size());
		EXPECT_EQ(2u, balances.size());
		EXPECT_EQ(Amount(46), balances.get(mosaicId1));
		EXPECT_EQ(Amount(99), balances.get(mosaicId2));
	}

	// endregion

	// region implicitly funded account

	TEST(TEST_CLASS, ObserverCanProcessImplicitlyFundedNemesisAccount) {
		// Arrange:
		auto mosaicId1 = MosaicId(1234);
		auto mosaicId2 = MosaicId(8844);

		// - don't pre-fund account
		test::AccountObserverTestContext context(observers::NotifyMode::Commit, Height(1));
		auto sender = test::GenerateRandomByteArray<Address>();

		NemesisFundingState fundingState;
		auto pObserver = CreateNemesisFundingObserver(sender, fundingState);
		auto notification1 = CreateBalanceTransferNotification(sender, test::UnresolveXor(mosaicId1), Amount(12));
		auto notification2 = CreateBalanceTransferNotification(sender, test::UnresolveXor(mosaicId1), Amount(34));
		auto notification3 = CreateBalanceTransferNotification(sender, test::UnresolveXor(mosaicId2), Amount(39));
		auto notification4 = CreateBalanceTransferNotification(sender, test::UnresolveXor(mosaicId2), Amount(60));

		// Act:
		test::ObserveNotification(*pObserver, notification1, context.observerContext()); // NemesisFundingType::Unknown
		test::ObserveNotification(*pObserver, notification2, context.observerContext()); // NemesisFundingType::Implicit
		test::ObserveNotification(*pObserver, notification3, context.observerContext()); // NemesisFundingType::Implicit
		test::ObserveNotification(*pObserver, notification4, context.observerContext()); // NemesisFundingType::Implicit

		// Assert:
		EXPECT_EQ(NemesisFundingType::Implicit, fundingState.FundingType);
		EXPECT_EQ(2u, fundingState.TotalFundedMosaics.size());
		EXPECT_EQ(Amount(46), fundingState.TotalFundedMosaics.get(mosaicId1));
		EXPECT_EQ(Amount(99), fundingState.TotalFundedMosaics.get(mosaicId2));

		auto& accountStateCache = context.cache().sub<cache::AccountStateCache>();
		auto accountStateIter = accountStateCache.find(sender);
		const auto& balances = accountStateIter.get().Balances;
		EXPECT_EQ(1u, accountStateCache.size());
		EXPECT_EQ(2u, balances.size());
		EXPECT_EQ(Amount(46), balances.get(mosaicId1));
		EXPECT_EQ(Amount(99), balances.get(mosaicId2));
	}

	// endregion
}}
