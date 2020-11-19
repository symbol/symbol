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

#include "mongo/src/AggregateExternalCacheStorage.h"
#include "mongo/tests/test/mocks/MockExternalCacheStorage.h"
#include "tests/test/cache/CacheTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace mongo {

#define TEST_CLASS AggregateExternalCacheStorageTests

	namespace {
		using StorageContainer = AggregateExternalCacheStorage::StorageContainer;
	}

	namespace {
		template<size_t CacheId>
		auto CreateStorageWithId() {
			return std::make_unique<mocks::MockExternalCacheStorage<CacheId>>();
		}
	}

	TEST(TEST_CLASS, CanCreateEmptyAggregateExternalCacheStorage) {
		// Act:
		AggregateExternalCacheStorage storage({});

		// Assert:
		EXPECT_EQ(0u, storage.id());
		EXPECT_EQ("{}", storage.name());
	}

	TEST(TEST_CLASS, CanCreateAggregateExternalCacheStorageAroundSingleStorage) {
		// Arrange:
		StorageContainer container;
		container.emplace_back(CreateStorageWithId<3>());

		// Act:
		AggregateExternalCacheStorage storage(std::move(container));

		// Assert:
		EXPECT_EQ(0u, storage.id());
		EXPECT_EQ("{ SimpleCache }", storage.name());
	}

	TEST(TEST_CLASS, CanCreateAggregateExternalCacheStorageAroundMultipleStorages) {
		// Arrange:
		StorageContainer container;
		container.emplace_back(CreateStorageWithId<3>());
		container.emplace_back(CreateStorageWithId<6>());
		container.emplace_back(CreateStorageWithId<4>());

		// Act:
		AggregateExternalCacheStorage storage(std::move(container));

		// Assert:
		EXPECT_EQ(0u, storage.id());
		EXPECT_EQ("{ SimpleCache, SimpleCache, SimpleCache }", storage.name());
	}

	namespace {
		auto CreateCatapultCache() {
			std::vector<std::unique_ptr<cache::SubCachePlugin>> subCaches(4);
			subCaches[1] = test::MakeSubCachePlugin<test::SimpleCacheT<1>, test::SimpleCacheStorageTraits>();
			subCaches[2] = test::MakeSubCachePlugin<test::SimpleCacheT<2>, test::SimpleCacheStorageTraits>();
			subCaches[3] = test::MakeSubCachePlugin<test::SimpleCacheT<3>, test::SimpleCacheStorageTraits>();
			return cache::CatapultCache(std::move(subCaches));
		}

		template<size_t CacheId>
		void AddStorage(std::vector<ExternalCacheStorage*>& subStorages, StorageContainer& container) {
			auto pSubStorage = CreateStorageWithId<CacheId>();
			subStorages.push_back(pSubStorage.get());
			container.emplace_back(std::move(pSubStorage));
		}

		auto CreateAggregateExternalCacheStorage(std::vector<ExternalCacheStorage*>& subStorages) {
			StorageContainer container;
			AddStorage<1>(subStorages, container);
			AddStorage<2>(subStorages, container);
			AddStorage<3>(subStorages, container);
			return std::make_unique<AggregateExternalCacheStorage>(std::move(container));
		}

		template<size_t CacheId>
		void AssertStorage(const ExternalCacheStorage& storage, size_t numExpectedSaves, Height expectedChainHeight) {
			const auto& mockStorage = static_cast<const mocks::MockExternalCacheStorage<CacheId>&>(storage);
			std::string message = "for cache with id " + std::to_string(mockStorage.id());
			EXPECT_EQ(numExpectedSaves, mockStorage.numSaveDeltaCalls()) << message;
			EXPECT_EQ(expectedChainHeight, mockStorage.chainHeight()) << message;
		}
	}

	TEST(TEST_CLASS, AggregateExternalCacheStorage_SaveDeltaDelegatesToAllSubStorages) {
		// Arrange:
		std::vector<ExternalCacheStorage*> subStorages;
		auto pStorage = CreateAggregateExternalCacheStorage(subStorages);
		auto catapultCache = CreateCatapultCache();
		auto delta = catapultCache.createDelta();

		// Act:
		pStorage->saveDelta(cache::CacheChanges(delta));

		// Assert:
		EXPECT_EQ(0u, pStorage->id());
		EXPECT_EQ("{ SimpleCache, SimpleCache, SimpleCache }", pStorage->name());
		AssertStorage<1>(*subStorages[0], 1, Height(0));
		AssertStorage<2>(*subStorages[1], 1, Height(0));
		AssertStorage<3>(*subStorages[2], 1, Height(0));
	}
}}
