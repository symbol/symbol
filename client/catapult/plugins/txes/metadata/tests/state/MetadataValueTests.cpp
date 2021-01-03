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

#include "src/state/MetadataValue.h"
#include "tests/TestHarness.h"

namespace catapult { namespace state {

#define TEST_CLASS MetadataValueTests

	// region constructor

	TEST(TEST_CLASS, CanCreateEmptyValue) {
		// Act:
		auto value = MetadataValue();

		// Assert:
		EXPECT_TRUE(value.empty());
		EXPECT_EQ(0u, value.size());
		EXPECT_FALSE(!!value.data());
	}

	// endregion

	// region canTrim

	namespace {
		bool CanTrim(const std::vector<uint8_t>& buffer1, const std::vector<uint8_t>& buffer2, size_t count) {
			auto value = MetadataValue();
			value.update(buffer1);
			return value.canTrim(buffer2, count);
		}
	}

	TEST(TEST_CLASS, CanTrimReturnsFalseWhenValueAndTrimBuffersAreDifferentSizes) {
		// Arrange:
		std::vector<uint8_t> buffer1{ 0x9A, 0xC7, 0x33, 0x18, 0xA7, 0xB0, 0x36 };
		std::vector<uint8_t> buffer2{ 0x9A, 0xC7, 0x33, 0x18, 0xA7, 0xB0 };

		// Act + Assert:
		EXPECT_FALSE(CanTrim(buffer1, buffer2, 1));
		EXPECT_FALSE(CanTrim(buffer2, buffer1, 1));
	}

	namespace {
		void RunCanTrimTest(bool expectedCanTrim, size_t bufferSize, size_t differenceIndex, size_t start, size_t end) {
			// Arrange:
			auto buffer1 = test::GenerateRandomVector(bufferSize);
			auto buffer2 = buffer1;
			buffer2[differenceIndex] ^= 0xFF;

			// Act + Assert:
			for (auto i = start; i <= end; ++i) {
				EXPECT_EQ(expectedCanTrim, CanTrim(buffer1, buffer2, i)) << i;
				EXPECT_EQ(expectedCanTrim, CanTrim(buffer2, buffer1, i)) << i;
			}
		}
	}

	TEST(TEST_CLASS, CanTrimReturnsTrueWhenValueAndTrimBuffersProduceAtLeastCountTrailingZeros) {
		RunCanTrimTest(true, 8, 3, 0, 4);
	}

	TEST(TEST_CLASS, CanTrimReturnsFalseWhenValueAndTrimBuffersProduceFewerThanCountTrailingZeros) {
		RunCanTrimTest(false, 8, 3, 5, 8);
	}

	TEST(TEST_CLASS, CanTrimReturnsFalseWhenValueAndTrimBuffersAreSmallerThanCount) {
		RunCanTrimTest(false, 8, 3, 9, 16);
	}

	// endregion

	// region update

	TEST(TEST_CLASS, CanSetValue) {
		// Arrange:
		std::vector<uint8_t> buffer{ 0x9A, 0xC7, 0x33, 0x18, 0xA7, 0xB0, 0x36 };

		auto value = MetadataValue();

		// Act:
		value.update(buffer);

		// Assert:
		EXPECT_FALSE(value.empty());
		EXPECT_EQ(7u, value.size());
		EXPECT_EQ_MEMORY(buffer.data(), value.data(), buffer.size());
	}

	TEST(TEST_CLASS, CanUpdateEqualLengthValue) {
		// Arrange:
		std::vector<uint8_t> buffer1{ 0x9A, 0xC7, 0x33, 0x18, 0xA7, 0xB0, 0x36 };
		std::vector<uint8_t> buffer2{ 0x9A ^ 0xD4, 0xC7 ^ 0x60, 0x33 ^ 0x82, 0x18 ^ 0xF8, 0xA7 ^ 0x78, 0xB0 ^ 0xFE, 0x36 ^ 0x78 };
		std::vector<uint8_t> expected{ 0xD4, 0x60, 0x82, 0xF8, 0x78, 0xFE, 0x78 };

		auto value = MetadataValue();
		value.update(buffer1);

		// Act:
		value.update(buffer2);

		// Assert:
		EXPECT_FALSE(value.empty());
		EXPECT_EQ(7u, value.size());
		EXPECT_EQ_MEMORY(expected.data(), value.data(), expected.size());
	}

	TEST(TEST_CLASS, CanUpdateShorterValue) {
		// Arrange:
		std::vector<uint8_t> buffer1{ 0x9A, 0xC7, 0x33, 0x18, 0xA7, 0xB0, 0x36 };
		std::vector<uint8_t> buffer2{ 0x9A ^ 0xD4, 0xC7 ^ 0x60, 0x33 ^ 0x82, 0x18 ^ 0xF8 };
		std::vector<uint8_t> expected{ 0xD4, 0x60, 0x82, 0xF8 };

		auto value = MetadataValue();
		value.update(buffer1);

		// Act:
		value.update(buffer2);

		// Assert:
		EXPECT_FALSE(value.empty());
		EXPECT_EQ(4u, value.size());
		EXPECT_EQ_MEMORY(expected.data(), value.data(), expected.size());
	}

	TEST(TEST_CLASS, CanUpdateLongerValue) {
		// Arrange:
		std::vector<uint8_t> buffer1{ 0x9A, 0xC7, 0x33, 0x18, 0xA7, 0xB0, 0x36 };
		std::vector<uint8_t> buffer2{
			0x9A ^ 0xD4, 0xC7 ^ 0x60, 0x33 ^ 0x82, 0x18 ^ 0xF8, 0xA7 ^ 0x78, 0xB0 ^ 0xFE, 0x36 ^ 0x78,
			0xE6, 0x9D, 0xD6
		};
		std::vector<uint8_t> expected{ 0xD4, 0x60, 0x82, 0xF8, 0x78, 0xFE, 0x78, 0xE6, 0x9D, 0xD6 };

		auto value = MetadataValue();
		value.update(buffer1);

		// Act:
		value.update(buffer2);

		// Assert:
		EXPECT_FALSE(value.empty());
		EXPECT_EQ(10u, value.size());
		EXPECT_EQ_MEMORY(expected.data(), value.data(), expected.size());
	}

	// endregion
}}
