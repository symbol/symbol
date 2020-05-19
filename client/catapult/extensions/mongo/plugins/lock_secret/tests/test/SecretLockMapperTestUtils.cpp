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

#include "SecretLockMapperTestUtils.h"
#include "mongo/src/mappers/MapperUtils.h"
#include "plugins/txes/lock_secret/src/state/SecretLockInfo.h"
#include "mongo/tests/test/MapperTestUtils.h"
#include "tests/TestHarness.h"

using namespace catapult::mongo::mappers;

namespace catapult { namespace test {

	void AssertEqualLockInfoData(const state::SecretLockInfo& lockInfo, const bsoncxx::document::view& dbLockInfo) {
		AssertEqualBaseLockInfoData(lockInfo, dbLockInfo);
		EXPECT_EQ(lockInfo.HashAlgorithm, static_cast<model::LockHashAlgorithm>(GetUint8(dbLockInfo, "hashAlgorithm")));
		EXPECT_EQ(lockInfo.Secret, GetHashValue(dbLockInfo, "secret"));
		EXPECT_EQ(lockInfo.RecipientAddress, GetAddressValue(dbLockInfo, "recipientAddress"));
		EXPECT_EQ(lockInfo.CompositeHash, GetHashValue(dbLockInfo, "compositeHash"));
	}
}}
