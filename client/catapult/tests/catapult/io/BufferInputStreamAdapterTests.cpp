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

#include "catapult/io/BufferInputStreamAdapter.h"
#include "tests/catapult/io/test/StreamTests.h"
#include "tests/test/core/mocks/MockMemoryStream.h"
#include "tests/TestHarness.h"

namespace catapult { namespace io {

#define TEST_CLASS BufferInputStreamAdapterTests

	// region basic stream tests

	namespace {
		using VectorInputStream = BufferInputStreamAdapter<std::vector<uint8_t>>;

		class BufferContext {
		public:
			explicit BufferContext(const char*)
			{}

			auto outputStream() {
				return std::make_unique<mocks::MockMemoryStream>(m_buffer);
			}

			auto inputStream() {
				return std::make_unique<VectorInputStream>(m_buffer);
			}

		private:
			std::vector<uint8_t> m_buffer;
		};
	}

	DEFINE_STREAM_TESTS(BufferContext)

	// endregion

	// region position tests

	TEST(TEST_CLASS, PositionIsInitiallyZero) {
		// Arrange:
		auto buffer = test::GenerateRandomVector(123);
		VectorInputStream stream(buffer);

		// Act + Assert:
		EXPECT_EQ(0u, stream.position());
		EXPECT_FALSE(stream.eof());
	}

	TEST(TEST_CLASS, ReadAdvancesPosition) {
		// Arrange:
		auto buffer = test::GenerateRandomVector(123);
		VectorInputStream stream(buffer);

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

	// endregion
}}
