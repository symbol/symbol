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
