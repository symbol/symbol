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

#pragma once
#include "catapult/cache/CacheConfiguration.h"
#include "tests/test/nodeps/Filesystem.h"
#include "tests/TestHarness.h"

namespace catapult { namespace test {

	/// Tests for a summary aware cache storage plugin.
	template<typename TTraits>
	class SummaryAwareCacheStoragePluginTests {
	public:
		static void AssertCanCreateCacheStorageViaPluginForFullStorage() {
			// Assert:
			AssertCanCreateStorageViaPlugin(cache::CacheConfiguration(), TTraits::Base_Name);
		}

		static void AssertCanCreateCacheStorageViaPluginForSummaryStorage() {
			// Arrange: use TempDirectoryGuard  to remove db directory without including rocksdb related includes
			test::TempDirectoryGuard dbDirGuard;

			auto cacheDatabaseConfig = config::NodeConfiguration::CacheDatabaseSubConfiguration();
			cacheDatabaseConfig.MaxWriteBatchSize = utils::FileSize::FromMegabytes(5);
			auto cacheConfig = cache::CacheConfiguration(dbDirGuard.name(), cacheDatabaseConfig, cache::PatriciaTreeStorageMode::Disabled);

			// Assert:
			AssertCanCreateStorageViaPlugin(cacheConfig, std::string(TTraits::Base_Name) + "_summary");
		}

	private:
		static void AssertCanCreateStorageViaPlugin(const cache::CacheConfiguration& config, const std::string& expectedStorageName) {
			// Arrange:
			typename TTraits::PluginType plugin(config);

			// Act:
			auto pStorage = plugin.createStorage();

			// Assert:
			ASSERT_TRUE(!!pStorage);
			EXPECT_EQ(expectedStorageName, pStorage->name());
		}
	};

#define MAKE_SUMMARY_AWARE_CACHE_STORAGE_PLUGIN_TEST(TRAITS, TEST_NAME) \
	TEST(TEST_CLASS, TEST_NAME) { \
		test::SummaryAwareCacheStoragePluginTests<TRAITS>::Assert##TEST_NAME(); \
	}

#define DEFINE_SUMMARY_AWARE_CACHE_STORAGE_PLUGIN_TESTS(TRAITS) \
	MAKE_SUMMARY_AWARE_CACHE_STORAGE_PLUGIN_TEST(TRAITS, CanCreateCacheStorageViaPluginForFullStorage) \
	MAKE_SUMMARY_AWARE_CACHE_STORAGE_PLUGIN_TEST(TRAITS, CanCreateCacheStorageViaPluginForSummaryStorage)
}}
