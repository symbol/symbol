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
#include "src/cache/MosaicCache.h"
#include "catapult/cache_core/AccountStateCache.h"
#include "tests/test/MosaicCacheTestUtils.h"
#include "tests/test/TransactionTestUtils.h"
#include "tests/test/plugins/ObserverTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace observers {

#define TEST_CLASS MosaicDefinitionObserverTests

	using ObserverTestContext = test::ObserverTestContextT<test::MosaicCacheFactory>;

	DEFINE_COMMON_OBSERVER_TESTS(MosaicDefinition,)

	namespace {
		constexpr NamespaceId Default_Namespace_Id(234);
		constexpr MosaicId Default_Mosaic_Id(345);
		constexpr uint8_t Default_Divisibility(17);

		model::MosaicDefinitionNotification CreateDefaultNotification(const Key& signer) {
			auto properties = model::MosaicProperties::FromValues({ { 0, Default_Divisibility, 0 } });
			return model::MosaicDefinitionNotification(signer, Default_Namespace_Id, Default_Mosaic_Id, properties);
		}

		template<typename TSeedCacheFunc, typename TCheckCacheFunc>
		void RunTest(
				const model::MosaicDefinitionNotification& notification,
				ObserverTestContext&& context,
				Amount initialOwnerBalance,
				TSeedCacheFunc seedCache,
				TCheckCacheFunc checkCache) {
			// Arrange:
			auto pObserver = CreateMosaicDefinitionObserver();

			// - seed the cache
			auto& accountStateCacheDelta = context.cache().sub<cache::AccountStateCache>();
			accountStateCacheDelta.addAccount(notification.Signer, Height(1));
			auto& ownerState = accountStateCacheDelta.find(notification.Signer).get();
			ownerState.Balances.credit(notification.MosaicId, initialOwnerBalance);

			auto& mosaicCacheDelta = context.cache().sub<cache::MosaicCache>();
			seedCache(mosaicCacheDelta);

			// Act:
			test::ObserveNotification(*pObserver, notification, context);

			// Assert: check the cache
			checkCache(mosaicCacheDelta, ownerState.Balances.get(notification.MosaicId));
		}

		void SeedCacheEmpty(cache::MosaicCacheDelta& mosaicCacheDelta) {
			// Sanity:
			test::AssertCacheContents(mosaicCacheDelta, {});
		}

		void SeedCacheWithDefaultMosaic(cache::MosaicCacheDelta& mosaicCacheDelta) {
			auto definition = state::MosaicDefinition(Height(7), Key(), model::MosaicProperties::FromValues({}));
			mosaicCacheDelta.insert(state::MosaicEntry(Default_Namespace_Id, Default_Mosaic_Id, definition));

			// Sanity:
			test::AssertCacheContents(mosaicCacheDelta, { Default_Mosaic_Id.unwrap() });
		}

		void AssertMosaicAdded(const cache::MosaicCacheDelta& mosaicCacheDelta, const Key& signer, Height height, size_t deepSize) {
			// Assert: the mosaic was added
			EXPECT_EQ(1u, mosaicCacheDelta.size());
			EXPECT_EQ(deepSize, mosaicCacheDelta.deepSize());
			ASSERT_TRUE(mosaicCacheDelta.contains(Default_Mosaic_Id));

			// - entry
			const auto& entry = mosaicCacheDelta.find(Default_Mosaic_Id).get();
			EXPECT_EQ(Default_Namespace_Id, entry.namespaceId());
			EXPECT_EQ(Default_Mosaic_Id, entry.mosaicId());

			// - definition
			const auto& definition = entry.definition();
			EXPECT_EQ(height, definition.height());
			EXPECT_EQ(signer, definition.owner());
			EXPECT_EQ(Default_Divisibility, definition.properties().divisibility()); // use divisibility as a proxy for checking properties

			// - supply + levy
			EXPECT_EQ(Amount(), entry.supply());
			EXPECT_FALSE(entry.hasLevy());
		}
	}

	// region commit

	TEST(TEST_CLASS, ObserverAddsMosaicOnCommit) {
		// Arrange:
		auto signer = test::GenerateRandomData<Key_Size>();
		auto notification = CreateDefaultNotification(signer);

		// Act: add it
		RunTest(
				notification,
				ObserverTestContext(NotifyMode::Commit, Height(888)),
				Amount(0), // owner balance must be zero because the mosaic does not yet exist
				SeedCacheEmpty,
				[&signer](const auto& mosaicCacheDelta, auto ownerBalance) {
					// Assert: the mosaic was added
					AssertMosaicAdded(mosaicCacheDelta, signer, Height(888), 1);

					// - the owner balance is zero
					EXPECT_EQ(Amount(0), ownerBalance);
				});
	}

	TEST(TEST_CLASS, ObserverOverwritesMosaicOnCommit) {
		// Arrange:
		auto signer = test::GenerateRandomData<Key_Size>();
		auto notification = CreateDefaultNotification(signer);

		// Act: add it
		RunTest(
				notification,
				ObserverTestContext(NotifyMode::Commit, Height(888)),
				Amount(1234),
				SeedCacheWithDefaultMosaic,
				[&signer](const auto& mosaicCacheDelta, auto ownerBalance) {
					// Assert: the mosaic was added
					AssertMosaicAdded(mosaicCacheDelta, signer, Height(888), 2);

					// - the owner balance is zero
					EXPECT_EQ(Amount(0), ownerBalance);
				});
	}

	// endregion

	// region rollback

	namespace {
		void AddTwoMosaics(cache::MosaicCacheDelta& mosaicCacheDelta, uint32_t supplyMultiplier) {
			auto definition = state::MosaicDefinition(Height(7), Key(), model::MosaicProperties::FromValues({}));
			for (auto id : { Default_Mosaic_Id, MosaicId(987) }) {
				auto entry = state::MosaicEntry(Default_Namespace_Id, id, definition);
				entry.increaseSupply(Amount((id.unwrap() + 100) * supplyMultiplier));
				mosaicCacheDelta.insert(entry);
			}
		}
	}

	TEST(TEST_CLASS, ObserverRemovesMosaicOnRollbackWhenHistoryDepthIsOne) {
		// Arrange:
		auto signer = test::GenerateRandomData<Key_Size>();
		auto notification = CreateDefaultNotification(signer);

		// Act: remove it
		RunTest(
				notification,
				ObserverTestContext(NotifyMode::Rollback, Height(888)),
				Amount(1234),
				[](auto& mosaicCacheDelta) {
					// Arrange: create a cache with two mosaics
					AddTwoMosaics(mosaicCacheDelta, 1);

					// Sanity:
					test::AssertCacheContents(mosaicCacheDelta, { Default_Mosaic_Id.unwrap(), 987 });
				},
				[](const auto& mosaicCacheDelta, auto ownerBalance) {
					// Assert: the mosaic was removed
					EXPECT_EQ(1u, mosaicCacheDelta.size());
					EXPECT_EQ(1u, mosaicCacheDelta.deepSize());
					EXPECT_FALSE(mosaicCacheDelta.contains(Default_Mosaic_Id));
					EXPECT_TRUE(mosaicCacheDelta.contains(MosaicId(987)));

					// - the owner balance is zero
					EXPECT_EQ(Amount(0), ownerBalance);
				});
	}

	TEST(TEST_CLASS, ObserverRemovesLastMosaicEntryOnRollbackWhenHistoryDepthIsGreaterThanOne) {
		// Arrange:
		auto signer = test::GenerateRandomData<Key_Size>();
		auto notification = CreateDefaultNotification(signer);

		// Act: remove it
		RunTest(
				notification,
				ObserverTestContext(NotifyMode::Rollback, Height(888)),
				Amount(1234),
				[](auto& mosaicCacheDelta) {
					// Arrange: create a cache with two mosaics with two levels each
					AddTwoMosaics(mosaicCacheDelta, 1);
					AddTwoMosaics(mosaicCacheDelta, 2);

					// Sanity:
					test::AssertCacheContents(mosaicCacheDelta, { Default_Mosaic_Id.unwrap(), 987 });
				},
				[](const auto& mosaicCacheDelta, auto ownerBalance) {
					// Assert: the mosaic was removed
					EXPECT_EQ(2u, mosaicCacheDelta.size());
					EXPECT_EQ(3u, mosaicCacheDelta.deepSize());
					EXPECT_TRUE(mosaicCacheDelta.contains(Default_Mosaic_Id));
					EXPECT_TRUE(mosaicCacheDelta.contains(MosaicId(987)));

					// - the owner balance is equal to the supply of the first mosaic entry
					EXPECT_EQ(Amount(100 + Default_Mosaic_Id.unwrap()), ownerBalance);
				});
	}

	// endregion
}}
