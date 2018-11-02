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

#include "src/cache/MosaicCacheSubCachePlugin.h"
#include "tests/test/MosaicTestUtils.h"
#include "tests/test/cache/SummaryAwareCacheStoragePluginTests.h"
#include "tests/test/core/mocks/MockMemoryStream.h"
#include "tests/TestHarness.h"

namespace catapult { namespace cache {

#define TEST_CLASS MosaicCacheSubCachePluginTests

	// region MosaicCacheSummaryCacheStorage - saveAll

	TEST(TEST_CLASS, CanSaveDeepSize) {
		// Arrange:
		auto config = CacheConfiguration();
		MosaicCache cache(config);
		MosaicCacheSummaryCacheStorage storage(cache);
		{
			// - insert three definitions for one mosaic and two definitions for another
			auto delta = cache.createDelta();
			delta->insert(test::CreateMosaicEntry(MosaicId(234), Amount(1)));
			delta->insert(test::CreateMosaicEntry(MosaicId(432), Amount(1)));
			delta->insert(test::CreateMosaicEntry(MosaicId(234), Amount(2)));
			delta->insert(test::CreateMosaicEntry(MosaicId(432), Amount(2)));
			delta->insert(test::CreateMosaicEntry(MosaicId(234), Amount(3)));
			cache.commit();
		}

		std::vector<uint8_t> buffer;
		mocks::MockMemoryStream stream("", buffer);

		// Act:
		storage.saveAll(stream);

		// Assert: all sizes were saved
		ASSERT_EQ(sizeof(uint64_t), buffer.size());

		auto deepSize = reinterpret_cast<uint64_t&>(*buffer.data());
		EXPECT_EQ(5u, deepSize);

		// - there was a single flush
		EXPECT_EQ(1u, stream.numFlushes());
	}

	TEST(TEST_CLASS, CanLoadDeepSize) {
		// Arrange:
		auto config = CacheConfiguration();
		MosaicCache cache(config);
		MosaicCacheSummaryCacheStorage storage(cache);

		std::vector<uint8_t> buffer;
		mocks::MockMemoryStream stream("", buffer);
		io::Write64(stream, 7);

		// Act:
		storage.loadAll(stream, 0);

		// Assert:
		auto view = cache.createView();
		EXPECT_EQ(7u, view->deepSize());
	}

	// endregion

	// region MosaicCacheSubCachePlugin

	namespace {
		struct PluginTraits {
			static constexpr auto Base_Name = "MosaicCache";
			using PluginType = MosaicCacheSubCachePlugin;
		};
	}

	DEFINE_SUMMARY_AWARE_CACHE_STORAGE_PLUGIN_TESTS(PluginTraits)

	// endregion
}}
