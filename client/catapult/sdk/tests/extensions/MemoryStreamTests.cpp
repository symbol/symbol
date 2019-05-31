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

#include "src/extensions/MemoryStream.h"
#include "tests/catapult/io/test/StreamTests.h"
#include "tests/TestHarness.h"

namespace catapult { namespace extensions {

#define TEST_CLASS MemoryStreamTests

	// region basic stream tests

	namespace {
		class MemoryStreamContext {
		public:
			explicit MemoryStreamContext(const char*)
			{}

			auto outputStream() {
				return std::make_unique<MemoryStream>(m_buffer);
			}

			auto inputStream() {
				return std::make_unique<MemoryStream>(m_buffer);
			}

		private:
			std::vector<uint8_t> m_buffer;
		};
	}

	DEFINE_STREAM_TESTS(MemoryStreamContext)

	// endregion

	// region position tests

	TEST(TEST_CLASS, PositionIsInitiallyZero) {
		// Arrange:
		auto buffer = test::GenerateRandomVector(123);
		MemoryStream stream(buffer);

		// Act + Assert:
		EXPECT_EQ(0u, stream.position());
		EXPECT_FALSE(stream.eof());
	}

	TEST(TEST_CLASS, ReadAdvancesPosition) {
		// Arrange:
		auto buffer = test::GenerateRandomVector(123);
		MemoryStream stream(buffer);

		// Act + Assert:
		std::vector<uint8_t> part(20);
		for (auto i = 1u; i <= 6; ++i) {
			stream.read(part);
			EXPECT_EQ(20u * i, stream.position());
			EXPECT_FALSE(stream.eof());
		}

		part.resize(3);
		stream.read(part);
		EXPECT_EQ(123u, stream.position());
		EXPECT_TRUE(stream.eof());

		// Sanity: end of stream is reached
		part.resize(1);
		EXPECT_THROW(stream.read(part), catapult_file_io_error);
	}

	TEST(TEST_CLASS, WriteDoesNotAdvancePosition) {
		// Arrange:
		auto buffer1 = test::GenerateRandomVector(123);
		MemoryStream stream(buffer1);

		// - read entire buffer to EOF
		std::vector<uint8_t> readBuffer1(123);
		stream.read(readBuffer1);

		// Sanity: stream is at EOF
		EXPECT_EQ(123u, stream.position());
		EXPECT_TRUE(stream.eof());

		// Act: write some data
		auto buffer2 = test::GenerateRandomVector(27);
		stream.write(buffer2);

		// Assert: stream is no longer at EOF
		EXPECT_EQ(123u, stream.position());
		EXPECT_FALSE(stream.eof());
	}

	// endregion

	// region read write tests

	TEST(TEST_CLASS, CanReadAfterWriteWhenPositionNotEof) {
		// Arrange:
		auto buffer1 = test::GenerateRandomVector(123);
		auto streamMemoryBuffer = buffer1;
		MemoryStream stream(streamMemoryBuffer);

		// - read part of buffer
		std::vector<uint8_t> readBuffer1(100);
		stream.read(readBuffer1);

		// Sanity: stream is not at EOF
		EXPECT_EQ(100u, stream.position());
		EXPECT_FALSE(stream.eof());

		// - write some data
		auto buffer2 = test::GenerateRandomVector(27);
		stream.write(buffer2);

		// Sanity: stream is still not at EOF
		EXPECT_EQ(100u, stream.position());
		EXPECT_FALSE(stream.eof());

		// Act: read more data
		std::vector<uint8_t> readBuffer2(50);
		stream.read(readBuffer2);

		// Assert: stream is at EOF
		EXPECT_EQ(150u, stream.position());
		EXPECT_TRUE(stream.eof());

		// - expected buffers were read
		EXPECT_EQ_MEMORY(buffer1.data(), readBuffer1.data(), readBuffer1.size());
		EXPECT_EQ_MEMORY(buffer1.data() + readBuffer1.size(), readBuffer2.data(), buffer1.size() - readBuffer1.size());
		EXPECT_EQ_MEMORY(buffer2.data(), readBuffer2.data() + buffer1.size() - readBuffer1.size(), buffer2.size());

		// - backing buffer was updated
		ASSERT_EQ(150u, streamMemoryBuffer.size());
		EXPECT_EQ_MEMORY(buffer1.data(), streamMemoryBuffer.data(), buffer1.size());
		EXPECT_EQ_MEMORY(buffer2.data(), streamMemoryBuffer.data() + buffer1.size(), buffer2.size());
	}

	TEST(TEST_CLASS, CanReadAfterWriteWhenPositionEof) {
		// Arrange:
		auto buffer1 = test::GenerateRandomVector(123);
		auto streamMemoryBuffer = buffer1;
		MemoryStream stream(streamMemoryBuffer);

		// - read entire buffer to EOF
		std::vector<uint8_t> readBuffer1(123);
		stream.read(readBuffer1);

		// Sanity: stream is at EOF
		EXPECT_EQ(123u, stream.position());
		EXPECT_TRUE(stream.eof());

		// - write some data
		auto buffer2 = test::GenerateRandomVector(27);
		stream.write(buffer2);

		// Sanity: stream is no longer at EOF
		EXPECT_EQ(123u, stream.position());
		EXPECT_FALSE(stream.eof());

		// Act: read more data
		std::vector<uint8_t> readBuffer2(27);
		stream.read(readBuffer2);

		// Assert: stream is at EOF
		EXPECT_EQ(150u, stream.position());
		EXPECT_TRUE(stream.eof());

		// - expected buffers were read
		EXPECT_EQ(buffer1, readBuffer1);
		EXPECT_EQ(buffer2, readBuffer2);

		// - backing buffer was updated
		ASSERT_EQ(150u, streamMemoryBuffer.size());
		EXPECT_EQ_MEMORY(buffer1.data(), streamMemoryBuffer.data(), buffer1.size());
		EXPECT_EQ_MEMORY(buffer2.data(), streamMemoryBuffer.data() + buffer1.size(), buffer2.size());
	}

	// endregion
}}
