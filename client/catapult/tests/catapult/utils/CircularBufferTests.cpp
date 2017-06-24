#include "catapult/utils/CircularBuffer.h"
#include "tests/TestHarness.h"

namespace catapult { namespace utils {

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

	TEST(CircularBufferTests, BufferIsInitiallyEmpty) {
		// Act:
		CircularBuffer<int> buffer(10);

		// Assert:
		EXPECT_EQ(0u, buffer.size());
		EXPECT_EQ(10u, buffer.capacity());
	}

	TEST(CircularBufferTests, CanAddFewerThanCapacityElementsToBuffer) {
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

	TEST(CircularBufferTests, CanAddLvaluesAndRvaluesToBuffer) {
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

	TEST(CircularBufferTests, CanAddCapacityElementsToBuffer) {
		// Act:
		CircularBuffer<int> buffer(7);
		PushAll(buffer, { 5, 7, 3, 2, 1, 4, 6 });

		// Assert:
		std::vector<int> expectedValues{ 5, 7, 3, 2, 1, 4, 6 };
		EXPECT_EQ(7u, buffer.size());
		EXPECT_EQ(7u, buffer.capacity());
		EXPECT_EQ(expectedValues, ToVector(buffer));
	}

	TEST(CircularBufferTests, PushBackWrapsAroundAfterAddingCapacityElementsToBuffer) {
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

	TEST(CircularBufferTests, PushBackCanWrapAroundMultipleTimes) {
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
			EXPECT_EQ(4u, value);
			EXPECT_FALSE(std::is_const<typename std::remove_reference<decltype(value)>::type>());

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
			EXPECT_EQ(3u, value);
			EXPECT_TRUE(std::is_const<typename std::remove_reference<decltype(value)>::type>());

			std::vector<int> expectedValues{ 5, 7, 3, 2 };
			EXPECT_EQ(expectedValues, ToVector(buffer));
		}
	}

	TEST(CircularBufferTests, CanRandomAccessElementInBuffer) {
		// Assert:
		AssertCanRandomAccessThirdElementInBuffer(2);
	}

	TEST(CircularBufferTests, CanRandomAccessElementInBufferWithWrapAroundIndex) {
		// Assert:
		AssertCanRandomAccessThirdElementInBuffer(10 + 2);
	}

	TEST(CircularBufferTests, CanRandomAccessElementInConstBuffer) {
		// Assert:
		AssertCanRandomAccessThirdElementInConstBuffer(2);
	}

	TEST(CircularBufferTests, CanRandomAccessElementInConstBufferWithWrapAroundIndex) {
		// Assert:
		AssertCanRandomAccessThirdElementInConstBuffer(10 + 2);
	}
}}
