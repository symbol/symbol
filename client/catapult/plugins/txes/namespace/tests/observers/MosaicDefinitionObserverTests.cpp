#include "src/observers/Observers.h"
#include "src/cache/MosaicCache.h"
#include "tests/test/MosaicCacheTestUtils.h"
#include "tests/test/TransactionTestUtils.h"
#include "tests/test/plugins/ObserverTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace observers {
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
				TSeedCacheFunc seedCache,
				TCheckCacheFunc checkCache) {
			// Arrange:
			auto pObserver = CreateMosaicDefinitionObserver();

			// - seed the cache
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
			const auto& entry = mosaicCacheDelta.get(Default_Mosaic_Id);
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

	TEST(MosaicDefinitionObserverTests, ObserverAddsMosaicOnCommit) {
		// Arrange:
		auto signer = test::GenerateRandomData<Key_Size>();
		auto notification = CreateDefaultNotification(signer);

		// Act: add it
		RunTest(
				notification,
				ObserverTestContext(NotifyMode::Commit, Height(888)),
				SeedCacheEmpty,
				[&signer](auto& mosaicCacheDelta) {
					// Assert: the mosaic was added
					AssertMosaicAdded(mosaicCacheDelta, signer, Height(888), 1);
				});
	}

	TEST(MosaicDefinitionObserverTests, ObserverOverwritesMosaicOnCommit) {
		// Arrange:
		auto signer = test::GenerateRandomData<Key_Size>();
		auto notification = CreateDefaultNotification(signer);

		// Act: add it
		RunTest(
				notification,
				ObserverTestContext(NotifyMode::Commit, Height(888)),
				SeedCacheWithDefaultMosaic,
				[&signer](auto& mosaicCacheDelta) {
					// Assert: the mosaic was added
					AssertMosaicAdded(mosaicCacheDelta, signer, Height(888), 2);
				});
	}

	// endregion

	// region rollback

	TEST(MosaicDefinitionObserverTests, ObserverRemovesMosaicOnRollback) {
		// Arrange:
		auto signer = test::GenerateRandomData<Key_Size>();
		auto notification = CreateDefaultNotification(signer);

		// Act: remove it
		RunTest(
				notification,
				ObserverTestContext(NotifyMode::Rollback, Height(888)),
				[](auto& mosaicCacheDelta) {
					// Arrange: create a cache with two mosaics
					auto definition = state::MosaicDefinition(Height(7), Key(), model::MosaicProperties::FromValues({}));
					for (auto id : { Default_Mosaic_Id, MosaicId(987) })
						mosaicCacheDelta.insert(state::MosaicEntry(Default_Namespace_Id, id, definition));

					// Sanity:
					test::AssertCacheContents(mosaicCacheDelta, { Default_Mosaic_Id.unwrap(), 987 });
				},
				[](auto& mosaicCacheDelta) {
					// Assert: the mosaic was removed
					EXPECT_EQ(1u, mosaicCacheDelta.size());
					EXPECT_FALSE(mosaicCacheDelta.contains(Default_Mosaic_Id));
					EXPECT_TRUE(mosaicCacheDelta.contains(MosaicId(987)));
				});
	}

	// endregion
}}
