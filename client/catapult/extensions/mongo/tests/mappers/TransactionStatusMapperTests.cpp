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

#include "mongo/src/mappers/TransactionStatusMapper.h"
#include "catapult/model/TransactionStatus.h"
#include "mongo/tests/test/MapperTestUtils.h"
#include "tests/TestHarness.h"
#include <mongocxx/client.hpp>

namespace catapult { namespace mongo { namespace mappers {

#define TEST_CLASS TransactionStatusMapperTests

	TEST(TEST_CLASS, CanMapTransactionStatus) {
		// Arrange:
		auto hash = test::GenerateRandomData<Hash256_Size>();
		auto status = 123456u;
		auto deadline = Timestamp(321);

		// Act:
		auto document = ToDbModel(model::TransactionStatus(hash, status, deadline));

		// Assert:
		auto view = document.view();
		EXPECT_EQ(3u, test::GetFieldCount(view));

		EXPECT_EQ(hash, test::GetHashValue(view, "hash"));
		EXPECT_EQ(123456u, test::GetUint32(view, "status"));
		EXPECT_EQ(321u, test::GetUint64(view, "deadline"));
	}
}}}
