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

#include "catapult/cache/CacheChangesSerializer.h"
#include "tests/catapult/cache/test/ByteVectorCacheChanges.h"
#include "tests/test/core/mocks/MockMemoryStream.h"
#include "tests/TestHarness.h"

namespace catapult { namespace cache {

#define TEST_CLASS CacheChangesSerializerTests

	// region ReadCacheChanges

	namespace {
		void WriteHeader(test::ByteVectorBufferWriter& writer, uint64_t numAdded, uint64_t numRemoved, uint64_t numCopied) {
			writer.write64(numAdded);
			writer.write64(numRemoved);
			writer.write64(numCopied);
		}

		void ReadFromBuffer(std::vector<uint8_t>& buffer, test::ByteVectorCacheChanges& changes) {
			// Act:
			mocks::MockMemoryStream outputStream(buffer);
			ReadCacheChanges<test::ByteVectorSerializer>(outputStream, changes);
		}
	}

	TEST(TEST_CLASS, CanReadCacheChanges_None) {
		// Arrange:
		std::vector<uint8_t> buffer;
		test::ByteVectorBufferWriter writer(buffer);
		WriteHeader(writer, 0, 0, 0);

		// Sanity:
		ASSERT_EQ(3u * sizeof(uint64_t), buffer.size());

		// Act:
		test::ByteVectorCacheChanges changes;
		ReadFromBuffer(buffer, changes);

		// Assert:
		EXPECT_EQ(0u, changes.Added.size());
		EXPECT_EQ(0u, changes.Removed.size());
		EXPECT_EQ(0u, changes.Copied.size());
	}

	TEST(TEST_CLASS, CanReadCacheChanges_OnlyAdded) {
		// Arrange:
		std::vector<uint8_t> buffer;
		test::ByteVectorBufferWriter writer(buffer);
		WriteHeader(writer, 2, 0, 0);
		auto added1 = writer.writeBuffer(12);
		auto added2 = writer.writeBuffer(44);

		// Sanity:
		ASSERT_EQ(5u * sizeof(uint64_t) + 12 + 44, buffer.size());

		// Act:
		test::ByteVectorCacheChanges changes;
		ReadFromBuffer(buffer, changes);

		// Assert:
		ASSERT_EQ(2u, changes.Added.size());
		EXPECT_EQ(0u, changes.Removed.size());
		EXPECT_EQ(0u, changes.Copied.size());

		EXPECT_EQ(added1, changes.Added[0]);
		EXPECT_EQ(added2, changes.Added[1]);
	}

	TEST(TEST_CLASS, CanReadCacheChanges_OnlyRemoved) {
		// Arrange:
		std::vector<uint8_t> buffer;
		test::ByteVectorBufferWriter writer(buffer);
		WriteHeader(writer, 0, 2, 0);
		auto removed1 = writer.writeBuffer(12);
		auto removed2 = writer.writeBuffer(44);

		// Sanity:
		ASSERT_EQ(5u * sizeof(uint64_t) + 12 + 44, buffer.size());

		// Act:
		test::ByteVectorCacheChanges changes;
		ReadFromBuffer(buffer, changes);

		// Assert:
		EXPECT_EQ(0u, changes.Added.size());
		ASSERT_EQ(2u, changes.Removed.size());
		EXPECT_EQ(0u, changes.Copied.size());

		EXPECT_EQ(removed1, changes.Removed[0]);
		EXPECT_EQ(removed2, changes.Removed[1]);
	}

	TEST(TEST_CLASS, CanReadCacheChanges_OnlyCopied) {
		// Arrange:
		std::vector<uint8_t> buffer;
		test::ByteVectorBufferWriter writer(buffer);
		WriteHeader(writer, 0, 0, 2);
		auto copied1 = writer.writeBuffer(12);
		auto copied2 = writer.writeBuffer(44);

		// Sanity:
		ASSERT_EQ(5u * sizeof(uint64_t) + 12 + 44, buffer.size());

		// Act:
		test::ByteVectorCacheChanges changes;
		ReadFromBuffer(buffer, changes);

		// Assert:
		EXPECT_EQ(0u, changes.Added.size());
		EXPECT_EQ(0u, changes.Removed.size());
		ASSERT_EQ(2u, changes.Copied.size());

		EXPECT_EQ(copied1, changes.Copied[0]);
		EXPECT_EQ(copied2, changes.Copied[1]);
	}

	TEST(TEST_CLASS, CanReadCacheChanges_All) {
		// Arrange:
		std::vector<uint8_t> buffer;
		test::ByteVectorBufferWriter writer(buffer);
		WriteHeader(writer, 1, 3, 2);
		auto added1 = writer.writeBuffer(12);
		auto removed1 = writer.writeBuffer(12);
		auto removed2 = writer.writeBuffer(44);
		auto removed3 = writer.writeBuffer(23);
		auto copied1 = writer.writeBuffer(12);
		auto copied2 = writer.writeBuffer(44);

		// Sanity:
		ASSERT_EQ(9u * sizeof(uint64_t) + 3 * 12 + 2 * 44 + 23, buffer.size());

		// Act:
		test::ByteVectorCacheChanges changes;
		ReadFromBuffer(buffer, changes);

		// Assert:
		ASSERT_EQ(1u, changes.Added.size());
		ASSERT_EQ(3u, changes.Removed.size());
		ASSERT_EQ(2u, changes.Copied.size());

		EXPECT_EQ(added1, changes.Added[0]);
		EXPECT_EQ(removed1, changes.Removed[0]);
		EXPECT_EQ(removed2, changes.Removed[1]);
		EXPECT_EQ(removed3, changes.Removed[2]);
		EXPECT_EQ(copied1, changes.Copied[0]);
		EXPECT_EQ(copied2, changes.Copied[1]);
	}

	// endregion

	// region WriteCacheChanges

	namespace {
		std::vector<uint8_t> WriteToBuffer(const test::ByteVectorSingleCacheChanges& changes) {
			// Act:
			std::vector<uint8_t> buffer;
			mocks::MockMemoryStream outputStream(buffer);
			WriteCacheChanges<test::ByteVectorSerializer>(outputStream, changes);
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
}}
