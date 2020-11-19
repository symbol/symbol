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

#include "mongo/src/ExternalCacheStorage.h"
#include "mongo/tests/test/mocks/MockExternalCacheStorage.h"
#include "tests/test/cache/CacheTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace mongo {

#define TEST_CLASS ExternalCacheStorageTests

	namespace {
		auto CreateCatapultCache() {
			std::vector<std::unique_ptr<cache::SubCachePlugin>> subCaches(2);
			subCaches[1] = test::MakeSubCachePlugin<test::SimpleCacheT<1>, test::SimpleCacheStorageTraits>();
			return cache::CatapultCache(std::move(subCaches));
		}

		template<size_t CacheId>
		void AssertStorage(const ExternalCacheStorage& storage, size_t numExpectedSaves, Height expectedChainHeight) {
			auto& mockStorage = static_cast<const mocks::MockExternalCacheStorage<CacheId>&>(storage);
			std::string message = "for cache with id " + std::to_string(mockStorage.id());
			EXPECT_EQ("SimpleCache", mockStorage.name()) << message;
			EXPECT_EQ(CacheId, mockStorage.id()) << message;
			EXPECT_EQ(numExpectedSaves, mockStorage.numSaveDeltaCalls()) << message;
			EXPECT_EQ(expectedChainHeight, mockStorage.chainHeight()) << message;
		}
	}

	// region ExternalCacheStorage

	TEST(TEST_CLASS, CanCreateExternalStorage) {
		// Act:
		mocks::MockExternalCacheStorage<10> storage;

		// Assert:
		AssertStorage<10>(storage, 0, Height(0));
	}

	TEST(TEST_CLASS, SaveDeltaDelegatesToInternalImplementation) {
		// Arrange:
		std::unique_ptr<ExternalCacheStorage> pStorage = std::make_unique<mocks::MockExternalCacheStorage<1>>();
		auto catapultCache = CreateCatapultCache();
		auto delta = catapultCache.createDelta();

		// Act:
		pStorage->saveDelta(cache::CacheChanges(delta));

		// Assert:
		AssertStorage<1>(*pStorage, 1, Height(0));
	}

	// endregion
}}
