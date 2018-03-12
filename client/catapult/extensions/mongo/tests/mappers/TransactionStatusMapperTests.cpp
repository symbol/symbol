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
