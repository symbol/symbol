#include "catapult/ionet/AppendContext.h"
#include "tests/TestHarness.h"

namespace catapult { namespace ionet {

	TEST(AppendContextTests, ConstructorResizesBuffer) {
		// Arrange:
		ByteBuffer buffer(12);
		AppendContext context(buffer, 100);

		// Assert:
		EXPECT_EQ(112u, buffer.size());
	}

	TEST(AppendContextTests, MutableBufferCanBeAccessedBeforeCommit) {
		// Arrange:
		ByteBuffer buffer(12);
		AppendContext context(buffer, 100);
		test::FillWithRandomData(buffer);

		// Assert:
		auto pContextBuffer = boost::asio::buffer_cast<uint8_t*>(context.buffer());
		auto contextBufferSize = boost::asio::buffer_size(context.buffer());
		EXPECT_EQ(100u, contextBufferSize);
		EXPECT_TRUE(std::equal(buffer.begin() + 12, buffer.end(), pContextBuffer, pContextBuffer + contextBufferSize));
	}

	TEST(AppendContextTests, MutableBufferCannotBeAccessedAfterCommit) {
		// Arrange:
		ByteBuffer buffer(12);
		AppendContext context(buffer, 100);
		context.commit(100);

		// Assert:
		EXPECT_THROW(context.buffer(), catapult_runtime_error);
	}

	TEST(AppendContextTests, CanCommitAllReservedData) {
		// Arrange:
		ByteBuffer buffer(12);
		AppendContext context(buffer, 100);

		// Act:
		context.commit(100);

		// Assert:
		EXPECT_EQ(112u, buffer.size());
	}

	TEST(AppendContextTests, CanCommitPartialReservedData) {
		// Arrange:
		ByteBuffer buffer(12);
		AppendContext context(buffer, 100);

		// Act:
		context.commit(63);

		// Assert:
		EXPECT_EQ(75u, buffer.size());
	}

	TEST(AppendContextTests, CannotCommitMoreDataThanReserved) {
		// Arrange:
		ByteBuffer buffer(12);
		AppendContext context(buffer, 100);

		// Act:
		EXPECT_THROW(context.commit(101), catapult_runtime_error);
	}

	TEST(AppendContextTests, CannotCommitDataMultipleTimes) {
		// Arrange:
		ByteBuffer buffer(12);
		AppendContext context(buffer, 100);

		// Act:
		context.commit(100);
		EXPECT_THROW(context.commit(100), catapult_runtime_error);
	}

	TEST(AppendContextTests, CanAbandonReservedData) {
		// Arrange:
		ByteBuffer buffer(12);

		// Act:
		{
			AppendContext context(buffer, 100);
		}

		// Assert:
		EXPECT_EQ(12u, buffer.size());
	}

	TEST(AppendContextTests, CanDestroyAfterCommitWithNoAbandonment) {
		// Arrange:
		ByteBuffer buffer(12);

		// Act:
		{
			AppendContext context(buffer, 100);
			context.commit(75);
		}

		// Assert:
		EXPECT_EQ(87u, buffer.size());
	}

	TEST(AppendContextTests, MoveDoesNotCauseAbandonment) {
		// Arrange:
		ByteBuffer buffer(12);

		// Act:
		auto context2 = [&buffer]() {
			AppendContext context(buffer, 100);
			return context;
		}();

		// Assert:
		EXPECT_EQ(112u, buffer.size());
	}
}}
