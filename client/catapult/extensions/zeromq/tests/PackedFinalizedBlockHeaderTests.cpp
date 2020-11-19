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

#include "zeromq/src/PackedFinalizedBlockHeader.h"
#include "tests/test/nodeps/Alignment.h"
#include "tests/TestHarness.h"

namespace catapult { namespace zeromq {

#define TEST_CLASS PackedFinalizedBlockHeaderTests

#define HEADER_FIELDS FIELD(Round) FIELD(Height) FIELD(Hash)

	TEST(TEST_CLASS, HeaderHasExpectedSize) {
		// Arrange:
		auto expectedSize = 0u;

#define FIELD(X) expectedSize += SizeOf32<decltype(PackedFinalizedBlockHeader::X)>();
		HEADER_FIELDS
#undef FIELD

		// Assert:
		EXPECT_EQ(expectedSize, sizeof(PackedFinalizedBlockHeader));
		EXPECT_EQ(48u, sizeof(PackedFinalizedBlockHeader));
	}

	TEST(TEST_CLASS, HeaderHasProperAlignment) {
#define FIELD(X) EXPECT_ALIGNED(PackedFinalizedBlockHeader, X);
		HEADER_FIELDS
#undef FIELD

		EXPECT_EQ(0u, sizeof(PackedFinalizedBlockHeader) % 8);
	}

#undef HEADER_FIELDS
}}
