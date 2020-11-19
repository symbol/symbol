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
#include "catapult/model/InflationCalculator.h"
#include "tests/test/MosaicCacheTestUtils.h"
#include "tests/test/core/NotificationTestUtils.h"
#include "tests/test/plugins/ObserverTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace observers {

#define TEST_CLASS MosaicSupplyInflationObserverTests

	using ObserverTestContext = test::ObserverTestContextT<test::MosaicCacheFactory>;

	DEFINE_COMMON_OBSERVER_TESTS(MosaicSupplyInflation, MosaicId(), model::InflationCalculator())

	namespace {
		void AssertNoSupplyChangeWhenNoInflation(NotifyMode mode) {
			// Arrange:
			static constexpr auto Currency_Mosaic_Id = MosaicId(123);

			// - create observer
			model::InflationCalculator calculator;
			calculator.add(Height(100), Amount(200));
			calculator.add(Height(101), Amount(400));
			calculator.add(Height(102), Amount(0));
			auto pObserver = CreateMosaicSupplyInflationObserver(Currency_Mosaic_Id, calculator);

			// - create notification
			auto notification = test::CreateBlockNotification();

			// - create context
			ObserverTestContext context(mode, Height(888));

			// Act:
			ASSERT_NO_THROW(test::ObserveNotification(*pObserver, notification, context));

			// Assert: zero inflation at specified height, so mosaic supply shouldn't have been modified
			const auto& mosaicCacheDelta = context.cache().sub<cache::MosaicCache>();
			EXPECT_FALSE(mosaicCacheDelta.contains(Currency_Mosaic_Id));
		}
	}

	TEST(TEST_CLASS, NoSupplyChangeWhenNoInflation_Commit) {
		AssertNoSupplyChangeWhenNoInflation(NotifyMode::Commit);
	}

	TEST(TEST_CLASS, NoSupplyChangeWhenNoInflation_Rollback) {
		AssertNoSupplyChangeWhenNoInflation(NotifyMode::Rollback);
	}

	namespace {
		void AssertSupplyChangeWhenInflation(NotifyMode mode, Amount expectedSupply) {
			// Arrange:
			static constexpr auto Currency_Mosaic_Id = MosaicId(123);

			// - create observer
			model::InflationCalculator calculator;
			calculator.add(Height(100), Amount(200));
			calculator.add(Height(101), Amount(400));
			calculator.add(Height(102), Amount(0));
			auto pObserver = CreateMosaicSupplyInflationObserver(Currency_Mosaic_Id, calculator);

			// - create notification
			auto notification = test::CreateBlockNotification();

			// - create context initialized with cache with a mosaic supply
			ObserverTestContext context(mode, Height(101));
			test::AddMosaic(context.cache(), Currency_Mosaic_Id, Height(1), Eternal_Artifact_Duration, Amount(1000));

			// Act:
			test::ObserveNotification(*pObserver, notification, context);

			// Assert: supply was adjusted for inflation
			const auto& mosaicCacheDelta = context.cache().sub<cache::MosaicCache>();
			EXPECT_EQ(expectedSupply, mosaicCacheDelta.find(Currency_Mosaic_Id).get().supply());
		}
	}

	TEST(TEST_CLASS, SupplyChangeWhenInflation_Commit) {
		AssertSupplyChangeWhenInflation(NotifyMode::Commit, Amount(1400));
	}

	TEST(TEST_CLASS, SupplyChangeWhenInflation_Rollback) {
		AssertSupplyChangeWhenInflation(NotifyMode::Rollback, Amount(600));
	}
}}
