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

#include "src/cache/MultisigCacheStorage.h"
#include "src/cache/MultisigCache.h"
#include "tests/test/MultisigTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace cache {

#define TEST_CLASS MultisigCacheStorageTests

	TEST(TEST_CLASS, CanLoadValueIntoCache) {
		// Arrange: create a random value to insert
		state::MultisigEntry originalEntry(test::GenerateRandomData<Key_Size>());
		originalEntry.setMinApproval(23);
		originalEntry.setMinRemoval(34);

		for (auto i = 0u; i < 3u; ++i)
			originalEntry.cosignatories().insert(test::GenerateRandomData<Key_Size>());

		for (auto i = 0u; i < 4u; ++i)
			originalEntry.multisigAccounts().insert(test::GenerateRandomData<Key_Size>());

		// Act:
		MultisigCache cache(CacheConfiguration{});
		{
			auto delta = cache.createDelta();
			MultisigCacheStorage::LoadInto(originalEntry, *delta);
			cache.commit();
		}

		// Assert: the cache contains the value
		auto view = cache.createView();
		EXPECT_EQ(1u, view->size());
		ASSERT_TRUE(view->contains(originalEntry.key()));
		const auto& loadedEntry = view->find(originalEntry.key()).get();

		// - the loaded cache value is correct
		test::AssertEqual(originalEntry, loadedEntry);
	}
}}
