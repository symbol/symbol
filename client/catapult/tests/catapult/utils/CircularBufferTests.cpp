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

#include "catapult/utils/CircularBuffer.h"
#include "tests/TestHarness.h"

namespace catapult { namespace utils {

#define TEST_CLASS CircularBufferTests

	namespace {
		std::vector<int> ToVector(const CircularBuffer<int>& buffer) {
			std::vector<int> values(buffer.size());
			for (auto i = 0u; i < buffer.size(); ++i)
				values[i] = buffer[i];
			return values;
		}

		void PushAll(CircularBuffer<int>& buffer, const std::vector<int>& values) {
			for (auto value : values)
				buffer.push_back(value);
		}
	}

	TEST(TEST_CLASS, BufferIsInitiallyEmpty) {
		// Act:
		CircularBuffer<int> buffer(10);

		// Assert:
		EXPECT_EQ(0u, buffer.size());
		EXPECT_EQ(10u, buffer.capacity());
	}

	TEST(TEST_CLASS, CanAddFewerThanCapacityElementsToBuffer) {
		// Arrange:
		CircularBuffer<int> buffer(10);

		// Act:
		buffer.push_back(5);
		buffer.push_back(7);
		buffer.push_back(3);
		buffer.push_back(2);

		// Assert:
		std::vector<int> expectedValues{ 5, 7, 3, 2 };
		EXPECT_EQ(4u, buffer.size());
		EXPECT_EQ(10u, buffer.capacity());
		EXPECT_EQ(expectedValues, ToVector(buffer));
	}

	TEST(TEST_CLASS, CanAddLvaluesAndRvaluesToBuffer) {
		// Arrange:
		CircularBuffer<int> buffer(10);

		// Act:
		int v1 = 8;
		int v2 = 9;
		buffer.push_back(5);
		buffer.push_back(v1);
		buffer.push_back(3);
		buffer.push_back(v2);

		// Assert:
		std::vector<int> expectedValues{ 5, 8, 3, 9 };
		EXPECT_EQ(4u, buffer.size());
		EXPECT_EQ(10u, buffer.capacity());
		EXPECT_EQ(expectedValues, ToVector(buffer));
	}

	TEST(TEST_CLASS, CanAddCapacityElementsToBuffer) {
		// Act:
		CircularBuffer<int> buffer(7);
		PushAll(buffer, { 5, 7, 3, 2, 1, 4, 6 });

		// Assert:
		std::vector<int> expectedValues{ 5, 7, 3, 2, 1, 4, 6 };
		EXPECT_EQ(7u, buffer.size());
		EXPECT_EQ(7u, buffer.capacity());
		EXPECT_EQ(expectedValues, ToVector(buffer));
	}

	TEST(TEST_CLASS, PushBackWrapsAroundAfterAddingCapacityElementsToBuffer) {
		// Arrange:
		CircularBuffer<int> buffer(7);
		PushAll(buffer, { 5, 7, 3, 2, 1, 4, 6 });

		// Act:
		PushAll(buffer, { 10 });

		// Assert:
		std::vector<int> expectedValues{ 10, 7, 3, 2, 1, 4, 6 };
		EXPECT_EQ(7u, buffer.size());
		EXPECT_EQ(7u, buffer.capacity());
		EXPECT_EQ(expectedValues, ToVector(buffer));
	}

	TEST(TEST_CLASS, PushBackCanWrapAroundMultipleTimes) {
		// Arrange:
		CircularBuffer<int> buffer(7);
		PushAll(buffer, { 5, 7, 3, 2, 1, 4, 6 });

		// Act:
		PushAll(buffer, { 10, 11, 12, 13, 14, 15, 16 });
		PushAll(buffer, { 30, 20 });

		// Assert:
		std::vector<int> expectedValues{ 30, 20, 12, 13, 14, 15, 16 };
		EXPECT_EQ(7u, buffer.size());
		EXPECT_EQ(7u, buffer.capacity());
		EXPECT_EQ(expectedValues, ToVector(buffer));
	}

	namespace {
		void AssertCanRandomAccessThirdElementInBuffer(size_t index) {
			// Arrange:
			CircularBuffer<int> buffer(10);
			PushAll(buffer, { 5, 7, 3, 2 });

			// Act: access and modify non-const element
			auto& value = ++buffer[index];

			// Assert:
			EXPECT_EQ(4, value);
			EXPECT_FALSE(std::is_const_v<std::remove_reference_t<decltype(value)>>);

			std::vector<int> expectedValues{ 5, 7, 4, 2 };
			EXPECT_EQ(expectedValues, ToVector(buffer));
		}

		void AssertCanRandomAccessThirdElementInConstBuffer(size_t index) {
			// Arrange:
			CircularBuffer<int> buffer(10);
			PushAll(buffer, { 5, 7, 3, 2 });

			// Act: access const element
			auto& value = const_cast<const CircularBuffer<int>&>(buffer)[index];

			// Assert:
			EXPECT_EQ(3, value);
			EXPECT_TRUE(std::is_const_v<std::remove_reference_t<decltype(value)>>);

			std::vector<int> expectedValues{ 5, 7, 3, 2 };
			EXPECT_EQ(expectedValues, ToVector(buffer));
		}
	}

	TEST(TEST_CLASS, CanRandomAccessElementInBuffer) {
		AssertCanRandomAccessThirdElementInBuffer(2);
	}

	TEST(TEST_CLASS, CanRandomAccessElementInBufferWithWrapAroundIndex) {
		AssertCanRandomAccessThirdElementInBuffer(10 + 2);
	}

	TEST(TEST_CLASS, CanRandomAccessElementInConstBuffer) {
		AssertCanRandomAccessThirdElementInConstBuffer(2);
	}

	TEST(TEST_CLASS, CanRandomAccessElementInConstBufferWithWrapAroundIndex) {
		AssertCanRandomAccessThirdElementInConstBuffer(10 + 2);
	}
}}
