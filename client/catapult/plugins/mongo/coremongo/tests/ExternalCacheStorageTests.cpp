#include "src/ExternalCacheStorage.h"
#include "tests/test/cache/CacheTestUtils.h"
#include "tests/test/mongo/mocks/MockExternalCacheStorage.h"
#include "tests/TestHarness.h"

#define TEST_CLASS ExternalCacheStorageTests

namespace catapult { namespace mongo { namespace plugins {

	namespace {
		auto CreateCatapultCache() {
			std::vector<std::unique_ptr<cache::SubCachePlugin>> subCaches(2);
			subCaches[1] = test::MakeSubCachePlugin<test::SimpleCacheT<1>, test::SimpleCacheStorageTraits>();
			return cache::CatapultCache(std::move(subCaches));
		}

		template<size_t CacheId>
		void AssertStorage(
				const ExternalCacheStorage& storage,
				size_t numExpectedSaves,
				size_t numExpectedLoads,
				Height expectedChainHeight) {
			auto& mockStorage = static_cast<const mocks::MockExternalCacheStorage<CacheId>&>(storage);
			std::string message = "for cache with id " + std::to_string(mockStorage.id());
			EXPECT_EQ("SimpleCache", mockStorage.name()) << message;
			EXPECT_EQ(CacheId, mockStorage.id()) << message;
			EXPECT_EQ(numExpectedSaves, mockStorage.numSaveDeltaCalls()) << message;
			EXPECT_EQ(numExpectedLoads, mockStorage.numLoadAllCalls()) << message;
			EXPECT_EQ(expectedChainHeight, mockStorage.chainHeight()) << message;
		}
	}

	// region ExternalCacheStorage

	TEST(TEST_CLASS, CanCreateExternalStorage) {
		// Act:
		mocks::MockExternalCacheStorage<10> storage;

		// Assert:
		AssertStorage<10>(storage, 0, 0, Height(0));
	}

	TEST(TEST_CLASS, SaveDeltaDelegatesToInternalImplementation) {
		// Arrange:
		std::unique_ptr<ExternalCacheStorage> pStorage = std::make_unique<mocks::MockExternalCacheStorage<1>>();
		auto catapultCache = CreateCatapultCache();
		auto delta = catapultCache.createDelta();

		// Act:
		pStorage->saveDelta(delta);

		// Assert:
		AssertStorage<1>(*pStorage, 1, 0, Height(0));
	}

	TEST(TEST_CLASS, LoadAllDelegatesToInternalImplementation) {
		// Arrange:
		std::unique_ptr<ExternalCacheStorage> pStorage = std::make_unique<mocks::MockExternalCacheStorage<1>>();
		auto catapultCache = CreateCatapultCache();

		// Act:
		pStorage->loadAll(catapultCache, Height(123));

		// Assert:
		AssertStorage<1>(*pStorage, 0, 1, Height(123));
	}

	// endregion
}}}
