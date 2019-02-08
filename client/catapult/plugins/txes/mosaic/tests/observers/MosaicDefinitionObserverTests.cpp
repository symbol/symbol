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
#include "src/state/MosaicLevy.h"
#include "catapult/cache_core/AccountStateCache.h"
#include "tests/test/MosaicCacheTestUtils.h"
#include "tests/test/plugins/ObserverTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace observers {

#define TEST_CLASS MosaicDefinitionObserverTests

	using ObserverTestContext = test::ObserverTestContextT<test::MosaicCacheFactory>;

	DEFINE_COMMON_OBSERVER_TESTS(MosaicDefinition,)

	namespace {
		constexpr MosaicId Default_Mosaic_Id(345);
		constexpr Height Seed_Height(7);

		model::MosaicDefinitionNotification CreateDefaultNotification(const Key& signer) {
			auto properties = model::MosaicProperties::FromValues({ { 3, 6, 15 } });
			return model::MosaicDefinitionNotification(signer, Default_Mosaic_Id, properties);
		}

		template<typename TSeedCacheFunc, typename TCheckCacheFunc>
		void RunTest(
				const model::MosaicDefinitionNotification& notification,
				ObserverTestContext&& context,
				TSeedCacheFunc seedCache,
				TCheckCacheFunc checkCache) {
			// Arrange:
			auto pObserver = CreateMosaicDefinitionObserver();

			auto& mosaicCacheDelta = context.cache().sub<cache::MosaicCache>();
			seedCache(mosaicCacheDelta);

			// Act:
			test::ObserveNotification(*pObserver, notification, context);

			// Assert: check the cache
			checkCache(mosaicCacheDelta);
		}

		void SeedCacheEmpty(cache::MosaicCacheDelta& mosaicCacheDelta) {
			// Sanity:
			test::AssertCacheContents(mosaicCacheDelta, {});
		}

		void SeedCacheWithDefaultMosaic(cache::MosaicCacheDelta& mosaicCacheDelta) {
			auto definition = state::MosaicDefinition(Seed_Height, Key(), 1, model::MosaicProperties::FromValues({ { 1, 2, 20 } }));
			mosaicCacheDelta.insert(state::MosaicEntry(Default_Mosaic_Id, definition));

			// Sanity:
			test::AssertCacheContents(mosaicCacheDelta, { Default_Mosaic_Id.unwrap() });
		}

		void AssertDefaultMosaic(
				const cache::MosaicCacheDelta& mosaicCacheDelta,
				const Key& signer,
				Height height,
				uint32_t expectedRevision,
				const model::MosaicProperties& expectedProperties) {
			// Assert: the mosaic was added
			ASSERT_TRUE(mosaicCacheDelta.contains(Default_Mosaic_Id));

			// - entry
			const auto& entry = mosaicCacheDelta.find(Default_Mosaic_Id).get();
			EXPECT_EQ(Default_Mosaic_Id, entry.mosaicId());

			// - definition
			const auto& definition = entry.definition();
			EXPECT_EQ(height, definition.height());
			EXPECT_EQ(signer, definition.owner());
			EXPECT_EQ(expectedRevision, definition.revision());
			EXPECT_EQ(expectedProperties, definition.properties());

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
		auto context = ObserverTestContext(NotifyMode::Commit, Height(888));
		RunTest(notification, std::move(context), SeedCacheEmpty, [&signer](const auto& mosaicCacheDelta) {
			// Assert: the mosaic was added
			EXPECT_EQ(1u, mosaicCacheDelta.size());
			AssertDefaultMosaic(mosaicCacheDelta, signer, Height(888), 1, model::MosaicProperties::FromValues({ { 3, 6, 15 } }));
		});
	}

	TEST(TEST_CLASS, ObserverOverwritesMosaicOnCommit) {
		// Arrange:
		auto signer = test::GenerateRandomData<Key_Size>();
		auto notification = CreateDefaultNotification(signer);

		// Act: add it
		auto context = ObserverTestContext(NotifyMode::Commit, Height(888));
		RunTest(notification, std::move(context), SeedCacheWithDefaultMosaic, [&signer](const auto& mosaicCacheDelta) {
			// Assert: the mosaic definition was changed
			// - height did not change
			// - required properties were xored
			// - duration was added
			EXPECT_EQ(1u, mosaicCacheDelta.size());
			AssertDefaultMosaic(mosaicCacheDelta, signer, Seed_Height, 2, model::MosaicProperties::FromValues({ { 2, 4, 20 + 15 } }));
		});
	}

	// endregion

	// region rollback

	namespace {
		void AddTwoMosaics(cache::MosaicCacheDelta& mosaicCacheDelta, uint32_t revision) {
			auto properties = model::MosaicProperties::FromValues({ { 2, 4, 20 + 15 } });
			auto definition = state::MosaicDefinition(Seed_Height, Key(), revision, properties);
			for (auto id : { Default_Mosaic_Id, MosaicId(987) })
				mosaicCacheDelta.insert(state::MosaicEntry(id, definition));

			// Sanity:
			test::AssertCacheContents(mosaicCacheDelta, { Default_Mosaic_Id.unwrap(), 987 });
		}
	}

	TEST(TEST_CLASS, ObserverRemovesMosaicOnRollbackWhenObserverDefinitionCounterIsEqualToOne) {
		// Arrange:
		auto signer = test::GenerateRandomData<Key_Size>();
		auto notification = CreateDefaultNotification(signer);
		auto seedMosaics = [](auto& mosaicCacheDelta) { AddTwoMosaics(mosaicCacheDelta, 1); };

		// Act: remove it
		auto context = ObserverTestContext(NotifyMode::Rollback, Seed_Height);
		RunTest(notification, std::move(context), seedMosaics, [](const auto& mosaicCacheDelta) {
			// Assert: the mosaic was removed
			EXPECT_EQ(1u, mosaicCacheDelta.size());
			EXPECT_FALSE(mosaicCacheDelta.contains(Default_Mosaic_Id));
			EXPECT_TRUE(mosaicCacheDelta.contains(MosaicId(987)));
		});
	}

	TEST(TEST_CLASS, ObserverDoesNotRemoveMosaicOnRollbackWhenDefinitionCounterIsGreaterThanOne) {
		// Arrange:
		auto signer = test::GenerateRandomData<Key_Size>();
		auto notification = CreateDefaultNotification(signer);
		auto seedMosaics = [](auto& mosaicCacheDelta) { AddTwoMosaics(mosaicCacheDelta, 2); };

		// Act: remove it
		auto context = ObserverTestContext(NotifyMode::Rollback, Seed_Height);
		RunTest(notification, std::move(context), seedMosaics, [&signer](const auto& mosaicCacheDelta) {
			// Assert: the mosaic was removed
			EXPECT_EQ(2u, mosaicCacheDelta.size());
			EXPECT_TRUE(mosaicCacheDelta.contains(Default_Mosaic_Id));
			EXPECT_TRUE(mosaicCacheDelta.contains(MosaicId(987)));

			// Assert: the default mosaic's definition was changed
			// - height did not change
			// - required properties were xored
			// - duration was subtracted
			AssertDefaultMosaic(mosaicCacheDelta, signer, Seed_Height, 1, model::MosaicProperties::FromValues({ { 1, 2, 20 } }));
		});
	}

	// endregion

	// region levy

	TEST(TEST_CLASS, ObserverThrowsWhenOverwritingMosaicWithLevy) {
		// Arrange:
		auto signer = test::GenerateRandomData<Key_Size>();
		auto notification = CreateDefaultNotification(signer);

		auto pObserver = CreateMosaicDefinitionObserver();
		auto context = ObserverTestContext(NotifyMode::Commit, Seed_Height);
		auto& mosaicCacheDelta = context.cache().sub<cache::MosaicCache>();
		auto definition = state::MosaicDefinition(Seed_Height, signer, 1, model::MosaicProperties::FromValues({ { 1, 2, 20 } }));
		auto entry = state::MosaicEntry(Default_Mosaic_Id, definition);
		auto address = test::GenerateRandomData<Address_Decoded_Size>();
		auto pLevy = std::make_unique<state::MosaicLevy>(Default_Mosaic_Id, address, std::vector<state::MosaicLevyRule>());
		entry.setLevy(std::move(pLevy));
		mosaicCacheDelta.insert(entry);

		// Act + Assert:
		EXPECT_THROW(test::ObserveNotification(*pObserver, notification, context), catapult_runtime_error);
	}

	// endregion
}}
