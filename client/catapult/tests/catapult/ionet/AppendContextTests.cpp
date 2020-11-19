/**
*** Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
*** Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
*** All rights reserved.
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

#include "catapult/ionet/AppendContext.h"
#include "tests/TestHarness.h"

namespace catapult { namespace ionet {

#define TEST_CLASS AppendContextTests

	// region constructor

	namespace {
		void AssertAppendBufferSize(size_t initialSize, size_t initialCapacity, size_t appendSize, size_t expectedSize) {
			// Arrange:
			ByteBuffer buffer(initialSize);
			buffer.reserve(initialCapacity);
			AppendContext context(buffer, appendSize);

			// Assert:
			EXPECT_EQ(expectedSize, buffer.size())
					<< "initialSize: " << initialSize
					<< ", initialCapacity: " << initialCapacity
					<< ", appendSize: " << appendSize;
		}
	}

	TEST(TEST_CLASS, ConstructorResizesBufferWhenLessThanHalfRequestedSizeIsAvailable) {
		AssertAppendBufferSize(26, 100, 150, 176); // (100 - 26) < 150 / 2
		AssertAppendBufferSize(12, 12, 100, 112);
	}

	TEST(TEST_CLASS, ConstructorDoesNotResizeBufferWhenAtLeastHalfRequestedSizeIsAvailable) {
		AssertAppendBufferSize(25, 100, 150, 100); // (100 - 25) == 150 / 2
		AssertAppendBufferSize(24, 100, 150, 100); // (100 - 24) > 150 / 2
		AssertAppendBufferSize(8, 100, 50, 58);
	}

	// endregion

	// region mutable buffer

	TEST(TEST_CLASS, MutableBufferCanBeAccessedBeforeCommitWhenBufferIsResized) {
		// Arrange:
		ByteBuffer buffer(12);
		AppendContext context(buffer, 100);
		test::FillWithRandomData(buffer);

		// Assert:
		auto pContextBuffer = boost::asio::buffer_cast<uint8_t*>(context.buffer());
		auto contextBufferSize = boost::asio::buffer_size(context.buffer());
		ASSERT_EQ(100u, contextBufferSize);
		EXPECT_TRUE(std::equal(buffer.begin() + 12, buffer.end(), pContextBuffer, pContextBuffer + contextBufferSize));
	}

	TEST(TEST_CLASS, MutableBufferCanBeAccessedBeforeCommitWhenBufferIsNotResized) {
		// Arrange:
		ByteBuffer buffer(8);
		buffer.reserve(100);
		AppendContext context(buffer, 50);
		test::FillWithRandomData(buffer);

		// Assert:
		auto pContextBuffer = boost::asio::buffer_cast<uint8_t*>(context.buffer());
		auto contextBufferSize = boost::asio::buffer_size(context.buffer());
		ASSERT_EQ(50u, contextBufferSize);
		EXPECT_TRUE(std::equal(buffer.begin() + 8, buffer.end(), pContextBuffer, pContextBuffer + contextBufferSize));
	}

	TEST(TEST_CLASS, MutableBufferCannotBeAccessedAfterCommit) {
		// Arrange:
		ByteBuffer buffer(12);
		AppendContext context(buffer, 100);
		context.commit(100);

		// Act + Assert:
		EXPECT_THROW(context.buffer(), catapult_runtime_error);
	}

	// endregion

	// region commit / abandon

	TEST(TEST_CLASS, CanCommitAllReservedData) {
		// Arrange:
		ByteBuffer buffer(12);
		AppendContext context(buffer, 100);

		// Act:
		context.commit(100);

		// Assert:
		EXPECT_EQ(112u, buffer.size());
	}

	TEST(TEST_CLASS, CanCommitPartialReservedData) {
		// Arrange:
		ByteBuffer buffer(12);
		AppendContext context(buffer, 100);

		// Act:
		context.commit(63);

		// Assert:
		EXPECT_EQ(75u, buffer.size());
	}

	TEST(TEST_CLASS, CannotCommitMoreDataThanReserved) {
		// Arrange:
		ByteBuffer buffer(12);
		AppendContext context(buffer, 100);

		// Act + Assert:
		EXPECT_THROW(context.commit(101), catapult_runtime_error);
	}

	TEST(TEST_CLASS, CannotCommitDataMultipleTimes) {
		// Arrange:
		ByteBuffer buffer(12);
		AppendContext context(buffer, 100);
		context.commit(100);

		// Act + Assert:
		EXPECT_THROW(context.commit(100), catapult_runtime_error);
	}

	TEST(TEST_CLASS, CanAbandonReservedData) {
		// Arrange:
		ByteBuffer buffer(12);

		// Act:
		{
			AppendContext context(buffer, 100);
		}

		// Assert:
		EXPECT_EQ(12u, buffer.size());
	}

	TEST(TEST_CLASS, CanDestroyAfterCommitWithNoAbandonment) {
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

	TEST(TEST_CLASS, MoveDoesNotCauseAbandonment) {
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

	// endregion
}}
