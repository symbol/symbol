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

#include "catapult/cache/ReadOnlyCatapultCache.h"
#include "catapult/state/CatapultState.h"
#include "tests/test/cache/SimpleCache.h"
#include "tests/TestHarness.h"

namespace catapult { namespace cache {

#define TEST_CLASS ReadOnlyCatapultCacheTests

	TEST(TEST_CLASS, CanCreateAroundArbitraryCaches) {
		// Arrange:
		state::CatapultState dependentState;

		test::SimpleCacheT<0> cache0;
		test::SimpleCacheT<2> cache2;
		std::vector<const void*> subViews{
			&cache0.createView()->asReadOnly(),
			nullptr,
			&cache2.createView()->asReadOnly()
		};

		// Act:
		ReadOnlyCatapultCache readOnlyCache(dependentState, subViews);

		// Assert:
		// - dependent state is held by reference
		EXPECT_EQ(&dependentState, &readOnlyCache.dependentState());

		// - sub caches match input
		EXPECT_EQ(subViews[0], &readOnlyCache.sub<test::SimpleCacheT<0>>());
		EXPECT_EQ(subViews[2], &readOnlyCache.sub<test::SimpleCacheT<2>>());
	}
}}
