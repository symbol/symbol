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

#include "catapult/cache/StateVersion.h"
#include "tests/test/core/mocks/MockMemoryStream.h"
#include "tests/TestHarness.h"

namespace catapult { namespace cache {

#define TEST_CLASS StateVersionTests

	namespace {
		struct MockTraits {
			static constexpr uint16_t State_Version = 42;
		};
	}

	TEST(TEST_CLASS, WriteAppendsVersionToOutput) {
		// Arrange:
		std::vector<uint8_t> buffer;
		mocks::MockMemoryStream outputStream(buffer);

		// Act:
		StateVersion<MockTraits>::Write(outputStream);

		// Assert:
		EXPECT_EQ(0u, outputStream.numFlushes());
		ASSERT_EQ(sizeof(uint16_t), buffer.size());
		EXPECT_EQ(MockTraits::State_Version, reinterpret_cast<const uint16_t&>(buffer[0]));
	}

	TEST(TEST_CLASS, ReadAndCheckFailsWhenVersionIsIncorrect) {
		// Arrange:
		std::vector<uint8_t> buffer(sizeof(uint16_t));
		reinterpret_cast<uint16_t&>(buffer[0]) = MockTraits::State_Version ^ 0xFFFF;
		mocks::MockMemoryStream inputStream(buffer);

		// Act + Assert:
		EXPECT_THROW(StateVersion<MockTraits>::ReadAndCheck(inputStream), catapult_runtime_error);
	}

	TEST(TEST_CLASS, ReadAndCheckSucceedsWhenVersionIsCorrect) {
		// Arrange:
		std::vector<uint8_t> buffer(sizeof(uint16_t));
		reinterpret_cast<uint16_t&>(buffer[0]) = MockTraits::State_Version;
		mocks::MockMemoryStream inputStream(buffer);

		// Act:
		StateVersion<MockTraits>::ReadAndCheck(inputStream);

		// Assert:
		EXPECT_EQ(sizeof(uint16_t), inputStream.position());
	}
}}
