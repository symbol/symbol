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

#include "src/cache/NamespaceCacheSubCachePlugin.h"
#include "tests/test/NamespaceCacheTestUtils.h"
#include "tests/test/NamespaceTestUtils.h"
#include "tests/test/cache/SummaryAwareCacheStoragePluginTests.h"
#include "tests/test/core/mocks/MockMemoryStream.h"
#include "tests/TestHarness.h"

namespace catapult { namespace cache {

#define TEST_CLASS NamespaceCacheSubCachePluginTests

	// region NamespaceCacheSummaryCacheStorage

	namespace {
		template<typename TAction>
		void RunCacheStorageTest(TAction action) {
			// Arrange:
			auto config = CacheConfiguration();
			NamespaceCache cache(config, NamespaceCacheTypes::Options());
			NamespaceCacheSummaryCacheStorage storage(cache);

			// Act + Assert:
			action(storage, cache);
		}
	}

	TEST(TEST_CLASS, CannotSaveAll) {
		// Arrange:
		RunCacheStorageTest([](const auto& storage, const auto&) {
			auto catapultCache = test::NamespaceCacheFactory::Create();
			auto cacheView = catapultCache.createView();

			std::vector<uint8_t> buffer;
			mocks::MockMemoryStream stream(buffer);

			// Act + Assert:
			EXPECT_THROW(storage.saveAll(cacheView, stream), catapult_invalid_argument);
		});
	}

	TEST(TEST_CLASS, CanSaveSummary) {
		// Arrange:
		RunCacheStorageTest([](const auto& storage, const auto&) {
			auto catapultCache = test::NamespaceCacheFactory::Create();

			// - insert root with 2 children, then renew root
			auto cacheDelta = catapultCache.createDelta();
			auto& delta = cacheDelta.sub<NamespaceCache>();
			auto owner = test::CreateRandomOwner();
			state::RootNamespace root(NamespaceId(123), owner, test::CreateLifetime(234, 321));
			delta.insert(root);
			delta.insert(state::Namespace(test::CreatePath({ 123, 127 })));
			delta.insert(state::Namespace(test::CreatePath({ 123, 128 })));
			state::RootNamespace renewedRoot(NamespaceId(123), owner, test::CreateLifetime(345, 456));
			delta.insert(renewedRoot);

			std::vector<uint8_t> buffer;
			mocks::MockMemoryStream stream(buffer);

			// Act:
			storage.saveSummary(cacheDelta, stream);

			// Assert: all sizes were saved
			ASSERT_EQ(sizeof(uint64_t) * 2, buffer.size());

			auto activeSize = reinterpret_cast<uint64_t&>(buffer[0]);
			auto deepSize = reinterpret_cast<uint64_t&>(buffer[sizeof(uint64_t)]);
			EXPECT_EQ(3u, activeSize);
			EXPECT_EQ(6u, deepSize);

			// - there was a single flush
			EXPECT_EQ(1u, stream.numFlushes());
		});
	}

	TEST(TEST_CLASS, CanLoadSummary) {
		// Arrange:
		RunCacheStorageTest([](auto& storage, const auto& cache) {
			std::vector<uint8_t> buffer;
			mocks::MockMemoryStream stream(buffer);
			io::Write64(stream, 7);
			io::Write64(stream, 11);
			stream.seek(0);

			// Act:
			storage.loadAll(stream, 0);

			// Assert:
			auto view = cache.createView();
			EXPECT_EQ(7u, view->activeSize());
			EXPECT_EQ(11u, view->deepSize());
		});
	}

	// endregion

	// region NamespaceCacheSubCachePlugin

	namespace {
		struct PluginTraits {
			static constexpr auto Base_Name = "NamespaceCache";

			class PluginType : public NamespaceCacheSubCachePlugin {
			public:
				explicit PluginType(const CacheConfiguration& config)
						: NamespaceCacheSubCachePlugin(config, NamespaceCacheTypes::Options())
				{}
			};
		};
	}

	DEFINE_SUMMARY_AWARE_CACHE_STORAGE_PLUGIN_TESTS(PluginTraits)

	// endregion
}}
