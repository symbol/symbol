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

#include "catapult/utils/FileSize.h"
#include "tests/test/nodeps/Comparison.h"
#include "tests/test/nodeps/Equality.h"
#include "tests/TestHarness.h"

namespace catapult { namespace utils {

#define TEST_CLASS FileSizeTests

	// region creation

	TEST(TEST_CLASS, CanCreateDefaultFileSize) {
		EXPECT_EQ(0u, FileSize().bytes());
	}

	TEST(TEST_CLASS, CanCreateFileSizeFromMegabytes) {
		EXPECT_EQ(1024 * 1024u, FileSize::FromMegabytes(1).bytes());
		EXPECT_EQ(2 * 1024 * 1024u, FileSize::FromMegabytes(2).bytes());
		EXPECT_EQ(10 * 1024 * 1024u, FileSize::FromMegabytes(10).bytes());
		EXPECT_EQ(123 * 1024 * 1024u, FileSize::FromMegabytes(123).bytes());
	}

	TEST(TEST_CLASS, CanCreateFileSizeFromKilobytes) {
		EXPECT_EQ(1024u, FileSize::FromKilobytes(1).bytes());
		EXPECT_EQ(2 * 1024u, FileSize::FromKilobytes(2).bytes());
		EXPECT_EQ(10 * 1024u, FileSize::FromKilobytes(10).bytes());
		EXPECT_EQ(123 * 1024u, FileSize::FromKilobytes(123).bytes());
	}

	TEST(TEST_CLASS, CanCreateFileSizeFromBytes) {
		EXPECT_EQ(1u, FileSize::FromBytes(1).bytes());
		EXPECT_EQ(2u, FileSize::FromBytes(2).bytes());
		EXPECT_EQ(10u, FileSize::FromBytes(10).bytes());
		EXPECT_EQ(123u, FileSize::FromBytes(123).bytes());
	}

	// endregion

	// region accessor conversions

	TEST(TEST_CLASS, MegabytesAreTruncatedWhenConverted) {
		constexpr uint64_t Base_Bytes = 10 * 1024 * 1024u;
		EXPECT_EQ(9u, FileSize::FromBytes(Base_Bytes - 1).megabytes());
		EXPECT_EQ(10u, FileSize::FromBytes(Base_Bytes).megabytes());
		EXPECT_EQ(10u, FileSize::FromBytes(Base_Bytes + 1).megabytes());
	}

	TEST(TEST_CLASS, KilobytesAreTruncatedWhenConverted) {
		constexpr uint64_t Base_Bytes = 10 * 1024u;
		EXPECT_EQ(9u, FileSize::FromBytes(Base_Bytes - 1).kilobytes());
		EXPECT_EQ(10u, FileSize::FromBytes(Base_Bytes).kilobytes());
		EXPECT_EQ(10u, FileSize::FromBytes(Base_Bytes + 1).kilobytes());
	}

	namespace {
		void Assert32BitFileSize(uint32_t value) {
			// Arrange:
			auto fileSize = FileSize::FromBytes(value);

			// Act + Assert: the value is accessible via bytes32 and bytes
			EXPECT_EQ(value, fileSize.bytes32());
			EXPECT_EQ(value, fileSize.bytes());
		}

		void Assert64BitFileSize(uint64_t value) {
			// Arrange:
			auto fileSize = FileSize::FromBytes(value);

			// Act + Assert: the value is accessible via bytes but not bytes32
			EXPECT_THROW(fileSize.bytes32(), catapult_runtime_error);
			EXPECT_EQ(value, fileSize.bytes());
		}
	}

	TEST(TEST_CLASS, Bytes32ReturnsBytesWhenBytes64FitsInto32Bit) {
		using NumericLimits = std::numeric_limits<uint32_t>;
		Assert32BitFileSize(NumericLimits::min()); // min
		Assert32BitFileSize(1); // other values
		Assert32BitFileSize(1234);
		Assert32BitFileSize(8692);
		Assert32BitFileSize(NumericLimits::max()); // max
	}

	TEST(TEST_CLASS, Bytes32ThrowsWhenBytes64DoesNotFitInto32Bit) {
		uint64_t max32 = std::numeric_limits<uint32_t>::max();
		Assert64BitFileSize(max32 + 1);
		Assert64BitFileSize(max32 + 1234);
		Assert64BitFileSize(max32 + 8692);
		Assert64BitFileSize(std::numeric_limits<uint64_t>::max());
	}

	// endregion

	// region equality operators

	namespace {
		std::unordered_set<std::string> GetEqualTags() {
			return { "10240 B", "10 KB", "10240 B (2)" };
		}

		std::unordered_map<std::string, FileSize> GenerateEqualityInstanceMap() {
			return {
				{ "10240 B", FileSize::FromBytes(10240) },
				{ "10 KB", FileSize::FromKilobytes(10) },
				{ "10240 B (2)", FileSize::FromBytes(10240) },

				{ "10239 B", FileSize::FromBytes(10239) },
				{ "10241 B", FileSize::FromBytes(10241) },
				{ "10240 KB", FileSize::FromKilobytes(10240) },
				{ "10 MB", FileSize::FromMegabytes(10) }
			};
		}
	}

	TEST(TEST_CLASS, OperatorEqualReturnsTrueOnlyForEqualValues) {
		test::AssertOperatorEqualReturnsTrueForEqualObjects("10240 B", GenerateEqualityInstanceMap(), GetEqualTags());
	}

	TEST(TEST_CLASS, OperatorNotEqualReturnsTrueOnlyForUnequalValues) {
		test::AssertOperatorNotEqualReturnsTrueForUnequalObjects("10240 B", GenerateEqualityInstanceMap(), GetEqualTags());
	}

	// endregion

	// region comparison operators

	namespace {
		std::vector<FileSize> GenerateIncreasingValues() {
			return {
				FileSize::FromBytes(10239),
				FileSize::FromKilobytes(10),
				FileSize::FromBytes(10241),
				FileSize::FromMegabytes(10),
				FileSize::FromKilobytes(10241)
			};
		}
	}

	DEFINE_COMPARISON_TESTS(TEST_CLASS, GenerateIncreasingValues())

	// endregion

	// region to string

	namespace {
		void AssertStringRepresentation(const std::string& expected, uint64_t numMegabytes, uint64_t numKilobytes, uint64_t numBytes) {
			// Arrange:
			auto fileSize = FileSize::FromBytes(((numMegabytes * 1024) + numKilobytes) * 1024 + numBytes);

			// Act:
			auto str = test::ToString(fileSize);

			// Assert:
			EXPECT_EQ(expected, str) << numMegabytes << "MB " << numKilobytes << "KB " << numBytes << "B";
		}
	}

	TEST(TEST_CLASS, CanOutputFileSize) {
		// Assert: zero
		AssertStringRepresentation("0B", 0, 0, 0);

		// - ones
		AssertStringRepresentation("1MB", 1, 0, 0);
		AssertStringRepresentation("1KB", 0, 1, 0);
		AssertStringRepresentation("1B", 0, 0, 1);

		// - overflows
		AssertStringRepresentation("20500MB", 20500, 0, 0);
		AssertStringRepresentation("1028MB", 1028, 0, 0);
		AssertStringRepresentation("1MB 4KB", 0, 1028, 0);
		AssertStringRepresentation("1KB 4B", 0, 0, 1028);

		// - all values
		AssertStringRepresentation("1MB 1KB 1B", 1, 1, 1);
		AssertStringRepresentation("1023MB 1023KB 1023B", 1023, 1023, 1023);
		AssertStringRepresentation("12MB 52KB 46B", 12, 52, 46);
		AssertStringRepresentation("12MB 46B", 12, 0, 46);
	}

	// endregion
}}
