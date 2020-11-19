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

#include "catapult/chain/ChainComparisonCode.h"
#include "tests/TestHarness.h"

namespace catapult { namespace chain {

#define TEST_CLASS ChainComparisonCodeTests

	TEST(TEST_CLASS, IsRemoteOutOfSyncOnlyReturnsTrueForOutOfSyncCodes) {
#define ENUM_VALUE(LABEL, VALUE) \
	EXPECT_EQ(0 != ((VALUE) & 0x40000000), IsRemoteOutOfSync(ChainComparisonCode::LABEL));

		CHAIN_COMPARISON_CODE_LIST

#undef ENUM_VALUE
	}

	TEST(TEST_CLASS, IsRemoteEvilOnlyReturnsTrueForEvilCodes) {
#define ENUM_VALUE(LABEL, VALUE) \
	EXPECT_EQ(0 != ((VALUE) & 0x80000000), IsRemoteEvil(ChainComparisonCode::LABEL));

		CHAIN_COMPARISON_CODE_LIST

#undef ENUM_VALUE
	}
}}
