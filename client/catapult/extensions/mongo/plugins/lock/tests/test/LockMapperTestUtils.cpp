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
