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

#include "catapult/cache_core/BlockStatisticCacheSubCachePlugin.h"
#include "catapult/model/BlockChainConfiguration.h"
#include "tests/test/cache/CacheTestUtils.h"
#include "tests/test/core/mocks/MockMemoryStream.h"
#include "tests/TestHarness.h"

namespace catapult { namespace cache {

#define TEST_CLASS BlockStatisticCacheSubCachePluginTests

	// region BlockStatisticCacheSummaryCacheStorage - saveAll / saveSummary

	namespace {
		void RunSaveConsistencyTest(size_t numValues) {
			// Arrange:
			auto catapultCache = test::CoreSystemCacheFactory::Create(model::BlockChainConfiguration::Uninitialized());
			{
				auto cacheDelta = catapultCache.createDelta();
				auto& delta = cacheDelta.sub<BlockStatisticCache>();
				for (auto i = 100u; i < 100 + numValues; ++i)
					delta.insert(state::BlockStatistic(Height(i), Timestamp(i), Difficulty(i), BlockFeeMultiplier(i)));

				catapultCache.commit(Height(7));

				// Sanity:
				EXPECT_EQ(numValues, delta.size());
			}

			BlockStatisticCacheSubCachePlugin plugin(111);
			auto pStorage = plugin.createStorage();

			// Act: serialize via saveAll
			std::vector<uint8_t> bufferAll;
			mocks::MockMemoryStream streamAll(bufferAll);
			pStorage->saveAll(catapultCache.createView(), streamAll);

			// - serialize via saveSummary
			std::vector<uint8_t> bufferSummary;
			mocks::MockMemoryStream streamSummary(bufferSummary);
			pStorage->saveSummary(catapultCache.createDelta(), streamSummary);

			// Assert:
			EXPECT_EQ(bufferAll, bufferSummary);
		}
	}

	TEST(TEST_CLASS, SaveAllAndSaveSummaryWriteSameData_ZeroValues) {
		RunSaveConsistencyTest(0);
	}

	TEST(TEST_CLASS, SaveAllAndSaveSummaryWriteSameData_SingleValue) {
		RunSaveConsistencyTest(1);
	}

	TEST(TEST_CLASS, SaveAllAndSaveSummaryWriteSameData_MultipleValues) {
		RunSaveConsistencyTest(11);
	}

	// endregion

	// region BlockStatisticCacheSubCachePlugin

	TEST(TEST_CLASS, CanCreateCacheStorageViaPlugin) {
		// Arrange:
		BlockStatisticCacheSubCachePlugin plugin(111);

		// Act:
		auto pStorage = plugin.createStorage();

		// Assert:
		ASSERT_TRUE(!!pStorage);
		EXPECT_EQ("BlockStatisticCache", pStorage->name());
	}

	// endregion
}}
