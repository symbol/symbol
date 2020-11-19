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

#include "catapult/cache/CacheChangesSerializer.h"
#include "tests/catapult/cache/test/ByteVectorCacheChanges.h"
#include "tests/test/core/SerializerTestUtils.h"
#include "tests/test/core/mocks/MockMemoryStream.h"
#include "tests/TestHarness.h"

namespace catapult { namespace cache {

#define TEST_CLASS CacheChangesSerializerTests

	// region WriteCacheChanges

	namespace {
		std::vector<uint8_t> WriteToBuffer(const test::ByteVectorSingleCacheChanges& changes) {
			// Act:
			std::vector<uint8_t> buffer;
			mocks::MockMemoryStream outputStream(buffer);
			WriteCacheChanges<test::ByteVectorSerializer>(changes, outputStream);
			return buffer;
		}
	}

	TEST(TEST_CLASS, CanWriteCacheChanges_None) {
		// Arrange:
		test::ByteVectorCacheChanges changes;

		// Act:
		auto buffer = WriteToBuffer(test::ByteVectorSingleCacheChanges(changes));

		// Assert:
		ASSERT_EQ(3u * sizeof(uint64_t), buffer.size());

		test::ByteVectorBufferReader reader(buffer);
		EXPECT_EQ(0u, reader.read64());
		EXPECT_EQ(0u, reader.read64());
		EXPECT_EQ(0u, reader.read64());
	}

	TEST(TEST_CLASS, CanWriteCacheChanges_OnlyAdded) {
		// Arrange:
		test::ByteVectorCacheChanges changes;
		changes.Added.push_back(test::GenerateRandomVector(21));
		changes.Added.push_back(test::GenerateRandomVector(14));

		// Act:
		auto buffer = WriteToBuffer(test::ByteVectorSingleCacheChanges(changes));

		// Assert:
		ASSERT_EQ(5u * sizeof(uint64_t) + 21 + 14, buffer.size());

		test::ByteVectorBufferReader reader(buffer);
		ASSERT_EQ(2u, reader.read64());
		EXPECT_EQ(0u, reader.read64());
		EXPECT_EQ(0u, reader.read64());

		test::AssertEquivalent(changes.Added, reader);
	}

	TEST(TEST_CLASS, CanWriteCacheChanges_OnlyRemoved) {
		// Arrange:
		test::ByteVectorCacheChanges changes;
		changes.Removed.push_back(test::GenerateRandomVector(21));
		changes.Removed.push_back(test::GenerateRandomVector(14));

		// Act:
		auto buffer = WriteToBuffer(test::ByteVectorSingleCacheChanges(changes));

		// Assert:
		ASSERT_EQ(5u * sizeof(uint64_t) + 21 + 14, buffer.size());

		test::ByteVectorBufferReader reader(buffer);
		EXPECT_EQ(0u, reader.read64());
		ASSERT_EQ(2u, reader.read64());
		EXPECT_EQ(0u, reader.read64());

		test::AssertEquivalent(changes.Removed, reader);
	}

	TEST(TEST_CLASS, CanWriteCacheChanges_OnlyCopied) {
		// Arrange:
		test::ByteVectorCacheChanges changes;
		changes.Copied.push_back(test::GenerateRandomVector(21));
		changes.Copied.push_back(test::GenerateRandomVector(14));

		// Act:
		auto buffer = WriteToBuffer(test::ByteVectorSingleCacheChanges(changes));

		// Assert:
		ASSERT_EQ(5u * sizeof(uint64_t) + 21 + 14, buffer.size());

		test::ByteVectorBufferReader reader(buffer);
		EXPECT_EQ(0u, reader.read64());
		EXPECT_EQ(0u, reader.read64());
		ASSERT_EQ(2u, reader.read64());

		test::AssertEquivalent(changes.Copied, reader);
	}

	TEST(TEST_CLASS, CanWriteCacheChanges_All) {
		// Arrange:
		test::ByteVectorCacheChanges changes;
		changes.Added.push_back(test::GenerateRandomVector(21));
		changes.Removed.push_back(test::GenerateRandomVector(21));
		changes.Removed.push_back(test::GenerateRandomVector(14));
		changes.Removed.push_back(test::GenerateRandomVector(17));
		changes.Copied.push_back(test::GenerateRandomVector(21));
		changes.Copied.push_back(test::GenerateRandomVector(14));

		// Act:
		auto buffer = WriteToBuffer(test::ByteVectorSingleCacheChanges(changes));

		// Assert:
		ASSERT_EQ(9u * sizeof(uint64_t) + 3 * 21 + 2 * 14 + 17, buffer.size());

		test::ByteVectorBufferReader reader(buffer);
		ASSERT_EQ(1u, reader.read64());
		ASSERT_EQ(3u, reader.read64());
		ASSERT_EQ(2u, reader.read64());

		test::AssertEquivalent(changes.Added, reader, "added");
		test::AssertEquivalent(changes.Removed, reader, "removed");
		test::AssertEquivalent(changes.Copied, reader, "copied");
	}

	// endregion

	// region Roundtrip

	namespace {
		void AssertEquivalent(
				const std::vector<std::vector<uint8_t>>& expectedBuffers,
				const std::vector<std::vector<uint8_t>>& actualBuffers,
				const std::string& message) {
			std::set<std::vector<uint8_t>> expectedBuffersSet(expectedBuffers.cbegin(), expectedBuffers.cend());
			std::set<std::vector<uint8_t>> actualBuffersSet(actualBuffers.cbegin(), actualBuffers.cend());

			EXPECT_EQ(expectedBuffersSet, actualBuffersSet) << message;
		}

		void RunRoundtripTest(const test::ByteVectorCacheChanges& originalChanges) {
			// Act:
			test::ByteVectorCacheChanges result;
			test::RunRoundtripBufferTest(
					test::ByteVectorSingleCacheChanges(originalChanges),
					result,
					WriteCacheChanges<test::ByteVectorSerializer, test::ByteVectorCacheDelta, std::vector<uint8_t>>,
					ReadCacheChanges<test::ByteVectorSerializer, std::vector<uint8_t>>);

			// Assert:
			AssertEquivalent(originalChanges.Added, result.Added, "added");
			AssertEquivalent(originalChanges.Removed, result.Removed, "removed");
			AssertEquivalent(originalChanges.Copied, result.Copied, "copied");
		}
	}

	TEST(TEST_CLASS, CanRoundtripCacheChanges_None) {
		// Arrange:
		test::ByteVectorCacheChanges changes;

		// Act + Assert:
		RunRoundtripTest(changes);
	}

	TEST(TEST_CLASS, CanRoundtripCacheChanges_OnlyAdded) {
		// Arrange:
		test::ByteVectorCacheChanges changes;
		changes.Added.push_back(test::GenerateRandomVector(21));
		changes.Added.push_back(test::GenerateRandomVector(14));

		// Act + Assert:
		RunRoundtripTest(changes);
	}

	TEST(TEST_CLASS, CanRoundtripCacheChanges_OnlyRemoved) {
		// Arrange:
		test::ByteVectorCacheChanges changes;
		changes.Removed.push_back(test::GenerateRandomVector(21));
		changes.Removed.push_back(test::GenerateRandomVector(14));

		// Act + Assert:
		RunRoundtripTest(changes);
	}

	TEST(TEST_CLASS, CanRoundtripCacheChanges_OnlyCopied) {
		// Arrange:
		test::ByteVectorCacheChanges changes;
		changes.Copied.push_back(test::GenerateRandomVector(21));
		changes.Copied.push_back(test::GenerateRandomVector(14));

		// Act + Assert:
		RunRoundtripTest(changes);
	}

	TEST(TEST_CLASS, CanRoundtripCacheChanges_All) {
		// Arrange:
		test::ByteVectorCacheChanges changes;
		changes.Added.push_back(test::GenerateRandomVector(21));
		changes.Removed.push_back(test::GenerateRandomVector(21));
		changes.Removed.push_back(test::GenerateRandomVector(14));
		changes.Removed.push_back(test::GenerateRandomVector(17));
		changes.Copied.push_back(test::GenerateRandomVector(21));
		changes.Copied.push_back(test::GenerateRandomVector(14));

		// Act + Assert:
		RunRoundtripTest(changes);
	}

	// endregion
}}
