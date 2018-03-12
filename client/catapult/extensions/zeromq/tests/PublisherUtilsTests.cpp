#include "zeromq/src/PublisherUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace zeromq {

#define TEST_CLASS PublisherUtilsTests

	TEST(TEST_CLASS, CanCreateTopic) {
		// Arrange:
		TransactionMarker marker = TransactionMarker(0x37);
		auto address = test::GenerateRandomData<Address_Decoded_Size>();

		// Act:
		auto topic = CreateTopic(marker, address);

		// Assert:
		ASSERT_EQ(Address_Decoded_Size + 1, topic.size());
		EXPECT_EQ(marker, TransactionMarker(topic[0]));
		EXPECT_TRUE(0 == std::memcmp(address.data(), topic.data() + 1, Address_Decoded_Size));
	}
}}
