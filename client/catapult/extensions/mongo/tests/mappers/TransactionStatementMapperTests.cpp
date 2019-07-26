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

#include "mongo/src/mappers/TransactionStatementMapper.h"
#include "catapult/model/TransactionStatement.h"
#include "mongo/tests/test/MapperTestUtils.h"
#include "mongo/tests/test/MongoReceiptTestUtils.h"
#include "mongo/tests/test/mocks/MockReceiptMapper.h"
#include "tests/TestHarness.h"

namespace catapult { namespace mongo { namespace mappers {

#define TEST_CLASS TransactionStatementMapperTests

	namespace {
		void AssertCanMapTransactionStatement(size_t numReceipts) {
			// Arrange:
			model::ReceiptSource source(123, 234);
			model::TransactionStatement statement(source);
			for (uint8_t i = 1; i <= numReceipts; ++i) {
				mocks::MockReceipt receipt;
				receipt.Size = sizeof(mocks::MockReceipt);
				receipt.Version = 345;
				receipt.Type = mocks::MockReceipt::Receipt_Type;
				receipt.Payload = { { i } };
				statement.addReceipt(receipt);
			}

			MongoReceiptRegistry registry;
			registry.registerPlugin(mocks::CreateMockReceiptMongoPlugin(utils::to_underlying_type(mocks::MockReceipt::Receipt_Type)));

			// Act:
			auto document = ToDbModel(Height(567), statement, registry);

			// Assert:
			test::AssertEqualTransactionStatement(statement, Height(567), document.view(), 3, 0);
		}
	}

	TEST(TEST_CLASS, CanMapTransactionStatement_NoReceipts) {
		AssertCanMapTransactionStatement(0);
	}

	TEST(TEST_CLASS, CanMapTransactionStatement_SingleReceipt) {
		AssertCanMapTransactionStatement(1);
	}

	TEST(TEST_CLASS, CanMapTransactionStatement_MultipleReceipts) {
		AssertCanMapTransactionStatement(5);
	}
}}}
