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

#include "src/cache/ReadOnlyMosaicRestrictionCache.h"
#include "src/cache/MosaicRestrictionCache.h"
#include "tests/TestHarness.h"

namespace catapult { namespace cache {

#define TEST_CLASS ReadOnlyMosaicRestrictionCacheTests

	TEST(TEST_CLASS, NetworkIdentifierIsExposed) {
		// Arrange:
		auto networkIdentifier = static_cast<model::NetworkIdentifier>(19);
		MosaicRestrictionCache originalCache(CacheConfiguration(), networkIdentifier);

		// Act + Assert:
		EXPECT_EQ(networkIdentifier, ReadOnlyMosaicRestrictionCache(*originalCache.createView()).networkIdentifier());
		EXPECT_EQ(networkIdentifier, ReadOnlyMosaicRestrictionCache(*originalCache.createDelta()).networkIdentifier());
	}
}}
