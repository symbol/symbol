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

#include "LockMapperTestUtils.h"
#include "mongo/src/mappers/MapperUtils.h"
#include "plugins/txes/lock/src/model/LockInfo.h"
#include "mongo/tests/test/MapperTestUtils.h"
#include "tests/TestHarness.h"

using namespace catapult::mongo::mappers;

namespace catapult { namespace test {

	namespace {
		void AssertEqualBaseLockInfoData(
				const model::LockInfo& lockInfo,
				const Address& accountAddress,
				const bsoncxx::document::view& dbLockInfo) {
			EXPECT_EQ(accountAddress, GetAddressValue(dbLockInfo, "accountAddress"));
			EXPECT_EQ(lockInfo.Account, GetKeyValue(dbLockInfo, "account"));
			EXPECT_EQ(lockInfo.MosaicId.unwrap(), GetUint64(dbLockInfo, "mosaicId"));
			EXPECT_EQ(lockInfo.Amount.unwrap(), GetUint64(dbLockInfo, "amount"));
			EXPECT_EQ(lockInfo.Height.unwrap(), GetUint64(dbLockInfo, "height"));
			EXPECT_EQ(lockInfo.Status, static_cast<model::LockStatus>(GetUint8(dbLockInfo, "status")));
		}
	}

	void AssertEqualLockInfoData(const model::HashLockInfo& lockInfo, const Address& address, const bsoncxx::document::view& dbLockInfo) {
		AssertEqualBaseLockInfoData(lockInfo, address, dbLockInfo);
		EXPECT_EQ(lockInfo.Hash, GetHashValue(dbLockInfo, "hash"));
	}

	void AssertEqualLockInfoData(
			const model::SecretLockInfo& lockInfo,
			const Address& address,
			const bsoncxx::document::view& dbLockInfo) {
		AssertEqualBaseLockInfoData(lockInfo, address, dbLockInfo);
		EXPECT_EQ(lockInfo.HashAlgorithm, static_cast<model::LockHashAlgorithm>(GetUint8(dbLockInfo, "hashAlgorithm")));
		EXPECT_EQ(lockInfo.Secret, GetHash512Value(dbLockInfo, "secret"));
		EXPECT_EQ(lockInfo.Recipient, GetAddressValue(dbLockInfo, "recipient"));
	}
}}
