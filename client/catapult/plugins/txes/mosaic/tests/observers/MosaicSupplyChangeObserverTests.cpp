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
#include "catapult/cache_core/AccountStateCache.h"
#include "tests/test/MosaicCacheTestUtils.h"
#include "tests/test/MosaicTestUtils.h"
#include "tests/test/plugins/ObserverTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace observers {

#define TEST_CLASS MosaicSupplyChangeObserverTests

	using ObserverTestContext = test::ObserverTestContextT<test::MosaicCacheFactory>;

	DEFINE_COMMON_OBSERVER_TESTS(MosaicSupplyChange,)

	namespace {
		constexpr MosaicId Default_Mosaic_Id(345);

		void AssertSupplyChange(
				model::MosaicSupplyChangeAction action,
				NotifyMode mode,
				Amount initialSupply,
				Amount initialOwnerSupply,
				Amount delta,
				Amount finalSupply,
				Amount finalOwnerSupply) {
			// Arrange: create observer and notification
			auto pObserver = CreateMosaicSupplyChangeObserver();

			auto owner = test::CreateRandomOwner();
			model::MosaicSupplyChangeNotification notification(owner, test::UnresolveXor(Default_Mosaic_Id), action, delta);

			// - initialize cache with a mosaic supply
			ObserverTestContext context(mode, Height(888));
			test::AddMosaic(context.cache(), Default_Mosaic_Id, Height(7), Eternal_Artifact_Duration, initialSupply);
			test::AddMosaicOwner(context.cache(), Default_Mosaic_Id, owner, initialOwnerSupply);

			// Act:
			test::ObserveNotification(*pObserver, notification, context);

			// Assert:
			const auto& mosaicCacheDelta = context.cache().sub<cache::MosaicCache>();
			EXPECT_EQ(finalSupply, mosaicCacheDelta.find(Default_Mosaic_Id).get().supply());

			const auto& accountStateCacheDelta = context.cache().sub<cache::AccountStateCache>();
			auto ownerAddress = accountStateCacheDelta.find(owner).get().Address;
			EXPECT_EQ(finalOwnerSupply, accountStateCacheDelta.find(ownerAddress).get().Balances.get(Default_Mosaic_Id));
		}

		void AssertSupplyIncrease(model::MosaicSupplyChangeAction action, NotifyMode mode) {
			// Assert:
			AssertSupplyChange(action, mode, Amount(500), Amount(222), Amount(123), Amount(500 + 123), Amount(222 + 123));
		}

		void AssertSupplyDecrease(model::MosaicSupplyChangeAction action, NotifyMode mode) {
			// Assert:
			AssertSupplyChange(action, mode, Amount(500), Amount(222), Amount(123), Amount(500 - 123), Amount(222 - 123));
		}
	}

	TEST(TEST_CLASS, IncreaseCommitIncreasesSupply) {
		AssertSupplyIncrease(model::MosaicSupplyChangeAction::Increase, NotifyMode::Commit);
	}

	TEST(TEST_CLASS, DecreaseCommitDecreasesSupply) {
		AssertSupplyDecrease(model::MosaicSupplyChangeAction::Decrease, NotifyMode::Commit);
	}

	TEST(TEST_CLASS, IncreaseRollbackDecreasesSupply) {
		AssertSupplyDecrease(model::MosaicSupplyChangeAction::Increase, NotifyMode::Rollback);
	}

	TEST(TEST_CLASS, DecreaseRollbackIncreasesSupply) {
		AssertSupplyIncrease(model::MosaicSupplyChangeAction::Decrease, NotifyMode::Rollback);
	}
}}
