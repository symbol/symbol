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

#include "src/cache/PropertyCacheStorage.h"
#include "src/cache/PropertyCache.h"
#include "src/model/PropertyTypes.h"
#include "tests/test/AccountPropertiesTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace cache {

#define TEST_CLASS PropertyCacheStorageTests

	TEST(TEST_CLASS, CanLoadValueIntoCache) {
		// Arrange:
		state::AccountProperties originalAccountProperties(test::GenerateRandomData<Address_Decoded_Size>());
		auto& property = originalAccountProperties.property(model::PropertyType::Address);
		for (auto i = 0u; i < 3; ++i)
			property.allow({ model::PropertyModificationType::Add, test::GenerateRandomVector(Address_Decoded_Size) });

		// Sanity:
		EXPECT_EQ(3u, originalAccountProperties.property<Address>(model::PropertyType::Address).size());

		// Act:
		PropertyCache cache(CacheConfiguration{}, model::NetworkIdentifier::Zero);
		{
			auto delta = cache.createDelta();
			PropertyCacheStorage::LoadInto(originalAccountProperties, *delta);
			cache.commit();
		}

		// Assert: the cache contains the value
		auto view = cache.createView();
		EXPECT_EQ(1u, view->size());
		ASSERT_TRUE(view->contains(originalAccountProperties.address()));
		const auto& loadedAccountProperties = view->find(originalAccountProperties.address()).get();

		// - the loaded cache value is correct
		test::AssertEqual(originalAccountProperties, loadedAccountProperties);
	}
}}
