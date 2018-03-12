#pragma once
#include "catapult/utils/MemoryUtils.h"
#include "mongo/tests/test/MapperTestUtils.h"
#include "tests/TestHarness.h"
#include <mongocxx/client.hpp>

namespace catapult {
	namespace model {
		struct HashLockInfo;
		struct SecretLockInfo;
	}
}

namespace catapult { namespace test {

	/// Assert that non-inherited fields of a lock \a transaction match with corresponding fields of \a dbTransaction.
	template<typename TTransaction>
	void AssertEqualNonInheritedLockTransactionData(const TTransaction& transaction, const bsoncxx::document::view& dbTransaction) {
		EXPECT_EQ(transaction.Duration.unwrap(), test::GetUint64(dbTransaction, "duration"));
		EXPECT_EQ(transaction.Mosaic.MosaicId.unwrap(), test::GetUint64(dbTransaction, "mosaicId"));
		EXPECT_EQ(transaction.Mosaic.Amount.unwrap(), test::GetUint64(dbTransaction, "amount"));
	}

	/// Verifies that db lock info (\a dbLockInfo) is equivalent to model hash lock info (\a lockInfo) and \a address.
	void AssertEqualLockInfoData(const model::HashLockInfo& lockInfo, const Address& address, const bsoncxx::document::view& dbLockInfo);

	/// Verifies that db lock info (\a dbLockInfo) is equivalent to model secret lock info (\a lockInfo) and \a address.
	void AssertEqualLockInfoData(const model::SecretLockInfo& lockInfo, const Address& address, const bsoncxx::document::view& dbLockInfo);
}}
