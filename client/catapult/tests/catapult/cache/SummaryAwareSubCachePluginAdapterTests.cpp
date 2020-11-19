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

#include "catapult/cache/SummaryAwareSubCachePluginAdapter.h"
#include "tests/test/cache/SimpleCache.h"
#include "tests/TestHarness.h"

namespace catapult { namespace cache {

#define TEST_CLASS SummaryAwareSubCachePluginAdapterTests

	namespace {
		class SimpleCacheSummaryCacheStorage : public SummaryCacheStorage<test::SimpleCache> {
		public:
			using SummaryCacheStorage<test::SimpleCache>::SummaryCacheStorage;

		public:
			void saveAll(const CatapultCacheView&, io::OutputStream&) const override {
				// do nothing
			}

			void saveSummary(const CatapultCacheDelta&, io::OutputStream&) const override {
				// do nothing
			}

			void loadAll(io::InputStream&, size_t) override {
				// do nothing
			}
		};

		void AssertCanCreateStorageViaPlugin(test::SimpleCacheViewMode mode, const std::string& expectedStorageName) {
			// Arrange:
			using PluginType = SummaryAwareSubCachePluginAdapter<
				test::SimpleCacheT<3>,
				test::SimpleCacheStorageTraits,
				SimpleCacheSummaryCacheStorage>;
			PluginType plugin(std::make_unique<test::SimpleCacheT<3>>(mode));

			// Act:
			auto pStorage = plugin.createStorage();

			// Assert:
			ASSERT_TRUE(!!pStorage);
			EXPECT_EQ(expectedStorageName, pStorage->name());
		}
	}

	TEST(TEST_CLASS, CanCreateCacheStorageViaPluginForFullStorage) {
		AssertCanCreateStorageViaPlugin(test::SimpleCacheViewMode::Iterable, "SimpleCache");
	}

	TEST(TEST_CLASS, CanCreateCacheStorageViaPluginForSummaryStorage) {
		AssertCanCreateStorageViaPlugin(test::SimpleCacheViewMode::Basic, "SimpleCache_summary");
	}
}}
