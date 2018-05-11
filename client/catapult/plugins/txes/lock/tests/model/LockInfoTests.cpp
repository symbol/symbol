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

#include "src/model/LockInfo.h"
#include "tests/TestHarness.h"

namespace catapult { namespace model {

#define TEST_CLASS LockInfoTests

	TEST(TEST_CLASS, IsActiveReturnsTrueIfHeightIsLessThanLockInfoHeight) {
		// Arrange:
		HashLockInfo lockInfo;
		lockInfo.Height = Height(123);

		// Act + Assert:
		EXPECT_TRUE(lockInfo.isActive(Height(122)));
		EXPECT_TRUE(lockInfo.isActive(Height(1)));
	}

	TEST(TEST_CLASS, IsActiveReturnsFalseIfHeightIsGreaterThanOrEqualToLockInfoHeight) {
		// Arrange:
		HashLockInfo lockInfo;
		lockInfo.Height = Height(123);

		// Act + Assert:
		EXPECT_FALSE(lockInfo.isActive(Height(123)));
		EXPECT_FALSE(lockInfo.isActive(Height(124)));
		EXPECT_FALSE(lockInfo.isActive(Height(1111)));
	}
}}
