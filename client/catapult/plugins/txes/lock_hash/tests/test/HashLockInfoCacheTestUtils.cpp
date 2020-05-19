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

#include "HashLockInfoCacheTestUtils.h"
#include "tests/test/nodeps/Random.h"

namespace catapult { namespace test {

	BasicHashLockInfoTestTraits::ValueType BasicHashLockInfoTestTraits::CreateLockInfo(Height height) {
		return state::HashLockInfo(
				GenerateRandomByteArray<Address>(),
				MosaicId(Random()),
				Amount(Random()),
				height,
				GenerateRandomByteArray<Hash256>());
	}

	BasicHashLockInfoTestTraits::ValueType BasicHashLockInfoTestTraits::CreateLockInfo() {
		return CreateLockInfo(test::GenerateRandomValue<Height>());
	}

	void BasicHashLockInfoTestTraits::SetKey(ValueType& lockInfo, const KeyType& key) {
		lockInfo.Hash = key;
	}

	void BasicHashLockInfoTestTraits::AssertEqual(const ValueType& lhs, const ValueType& rhs) {
		test::AssertEqualLockInfo(lhs, rhs);
		EXPECT_EQ(lhs.Hash, rhs.Hash);
	}
}}
