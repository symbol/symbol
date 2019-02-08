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

#include "src/cache/MosaicCacheStorage.h"
#include "src/cache/MosaicCache.h"
#include "tests/test/MosaicTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace cache {

#define TEST_CLASS MosaicCacheStorageTests

	TEST(TEST_CLASS, CanLoadValueIntoCache) {
		// Arrange: create an entry with optional properties
		auto properties = test::CreateMosaicPropertiesWithDuration(BlockDuration(37));
		auto definition = state::MosaicDefinition(Height(11), test::GenerateRandomData<Key_Size>(), 3, properties);
		auto originalEntry = state::MosaicEntry(MosaicId(680), definition);

		// Act:
		MosaicCache cache(CacheConfiguration{});
		auto delta = cache.createDelta();
		MosaicCacheStorage::LoadInto(originalEntry, *delta);
		cache.commit();

		// Assert: the cache contains the value
		auto view = cache.createView();
		EXPECT_EQ(1u, view->size());
		ASSERT_TRUE(view->contains(originalEntry.mosaicId()));
		const auto& loadedEntry = view->find(originalEntry.mosaicId()).get();

		// - the loaded cache value is correct
		test::AssertEqual(originalEntry, loadedEntry);
	}
}}
