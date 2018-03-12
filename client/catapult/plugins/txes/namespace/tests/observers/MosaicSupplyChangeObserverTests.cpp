#include "src/observers/Observers.h"
#include "src/cache/MosaicCache.h"
#include "catapult/cache_core/AccountStateCache.h"
#include "tests/test/MosaicCacheTestUtils.h"
#include "tests/test/plugins/ObserverTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace observers {

#define TEST_CLASS MosaicSupplyChangeObserverTests

	using ObserverTestContext = test::ObserverTestContextT<test::MosaicCacheFactory>;

	DEFINE_COMMON_OBSERVER_TESTS(MosaicSupplyChange,)

	namespace {
		constexpr MosaicId Default_Mosaic_Id(345);

		void AssertSupplyChange(
				model::MosaicSupplyChangeDirection direction,
				NotifyMode mode,
				Amount initialSupply,
				Amount initialOwnerSupply,
				Amount delta,
				Amount finalSupply,
				Amount finalOwnerSupply) {
			// Arrange: create observer and notification
			auto pObserver = CreateMosaicSupplyChangeObserver();

			auto signer = test::GenerateRandomData<Key_Size>();
			model::MosaicSupplyChangeNotification notification(signer, Default_Mosaic_Id, direction, delta);

			// - initialize cache with a mosaic supply
			ObserverTestContext context(mode, Height(888));
			test::AddMosaic(context.cache(), Default_Mosaic_Id, Height(7), Eternal_Artifact_Duration, initialSupply);
			test::AddMosaicOwner(context.cache(), Default_Mosaic_Id, signer, initialOwnerSupply);

			// Act:
			test::ObserveNotification(*pObserver, notification, context);

			// Assert:
			const auto& mosaicCacheDelta = context.cache().sub<cache::MosaicCache>();
			EXPECT_EQ(finalSupply, mosaicCacheDelta.get(Default_Mosaic_Id).supply());

			const auto& accountStateCacheDelta = context.cache().sub<cache::AccountStateCache>();
			auto signerAddress = accountStateCacheDelta.get(signer).Address;
			EXPECT_EQ(finalOwnerSupply, accountStateCacheDelta.get(signerAddress).Balances.get(Default_Mosaic_Id));
		}

		void AssertSupplyIncrease(model::MosaicSupplyChangeDirection direction, NotifyMode mode) {
			// Assert:
			AssertSupplyChange(direction, mode, Amount(500), Amount(222), Amount(123), Amount(500 + 123), Amount(222 + 123));
		}

		void AssertSupplyDecrease(model::MosaicSupplyChangeDirection direction, NotifyMode mode) {
			// Assert:
			AssertSupplyChange(direction, mode, Amount(500), Amount(222), Amount(123), Amount(500 - 123), Amount(222 - 123));
		}
	}

	TEST(TEST_CLASS, IncreaseCommitIncreasesSupply) {
		// Assert:
		AssertSupplyIncrease(model::MosaicSupplyChangeDirection::Increase, NotifyMode::Commit);
	}

	TEST(TEST_CLASS, DecreaseCommitDecreasesSupply) {
		// Assert:
		AssertSupplyDecrease(model::MosaicSupplyChangeDirection::Decrease, NotifyMode::Commit);
	}

	TEST(TEST_CLASS, IncreaseRollbackDecreasesSupply) {
		// Assert:
		AssertSupplyDecrease(model::MosaicSupplyChangeDirection::Increase, NotifyMode::Rollback);
	}

	TEST(TEST_CLASS, DecreaseRollbackIncreasesSupply) {
		// Assert:
		AssertSupplyIncrease(model::MosaicSupplyChangeDirection::Decrease, NotifyMode::Rollback);
	}
}}
