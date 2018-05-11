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

#include "catapult/cache/ChunkedDataLoader.h"
#include "tests/catapult/cache/test/CacheSerializationTestUtils.h"
#include "tests/test/core/mocks/MockMemoryStream.h"
#include "tests/TestHarness.h"

namespace catapult { namespace cache {

#define TEST_CLASS ChunkedDataLoaderTests

	namespace {
		using TestEntry = test::CacheSerializationTestEntry;
		using TestEntryLoaderTraits = test::CacheSerializationTestEntryLoaderTraits;

		constexpr auto GenerateRandomEntries = test::GenerateRandomCacheSerializationTestEntries;
		constexpr auto CopyEntriesToStreamBuffer = test::CopyCacheSerializationTestEntriesToStreamBuffer;
	}

	// region valid stream

	TEST(TEST_CLASS, CanLoadStorageFromEmptyStream) {
		// Arrange:
		auto buffer = CopyEntriesToStreamBuffer({});
		mocks::MockMemoryStream stream("", buffer);
		ChunkedDataLoader<TestEntryLoaderTraits> loader(stream);

		// Assert:
		EXPECT_FALSE(loader.hasNext());
	}

	namespace {
		void AssertCanLoadAllInOneShot(size_t numEntries, size_t numRequestedEntries) {
			// Arrange:
			auto seed = GenerateRandomEntries(numEntries);
			auto buffer = CopyEntriesToStreamBuffer(seed);
			mocks::MockMemoryStream stream("", buffer);
			ChunkedDataLoader<TestEntryLoaderTraits> loader(stream);

			// Sanity:
			EXPECT_TRUE(loader.hasNext());

			// Act:
			std::vector<TestEntry> loadedEntries;
			loader.next(numRequestedEntries, loadedEntries);

			// Assert:
			EXPECT_FALSE(loader.hasNext());
			EXPECT_EQ(seed, loadedEntries);
		}
	}

	TEST(TEST_CLASS, CanLoadStorageFromNonEmptyStreamInOneShot) {
		// Assert:
		AssertCanLoadAllInOneShot(7, 7);
	}

	TEST(TEST_CLASS, CanLoadNoMoreThanRemainingAvailableEntries) {
		// Assert: requesting 100 entries when only 7 are available should return all 7
		AssertCanLoadAllInOneShot(7, 100);
	}

	TEST(TEST_CLASS, CanLoadStorageFromNonEmptyStreamInMultipleBatches) {
		// Arrange:
		auto seed = GenerateRandomEntries(7);
		auto buffer = CopyEntriesToStreamBuffer(seed);
		mocks::MockMemoryStream stream("", buffer);
		ChunkedDataLoader<TestEntryLoaderTraits> loader(stream);

		// Act:
		auto offset = 0u;
		for (auto count : { 2u, 3u, 2u }) {
			// Sanity:
			EXPECT_TRUE(loader.hasNext());

			// Act:
			std::vector<TestEntry> loadedEntries;
			loader.next(count, loadedEntries);

			// Assert:
			auto expectedEntries = std::vector<TestEntry>(seed.cbegin() + offset, seed.cbegin() + offset + count);
			EXPECT_EQ(count, loadedEntries.size());
			EXPECT_EQ(expectedEntries, loadedEntries) << "offset " << offset << ", count " << count;
			offset += count;
		}

		// Assert:
		EXPECT_FALSE(loader.hasNext());
	}

	TEST(TEST_CLASS, ReadingFromEndOfStreamHasNoEffect) {
		// Arrange:
		auto buffer = CopyEntriesToStreamBuffer({});
		mocks::MockMemoryStream stream("", buffer);
		ChunkedDataLoader<TestEntryLoaderTraits> loader(stream);

		// Sanity:
		EXPECT_FALSE(loader.hasNext());

		// Act:
		std::vector<TestEntry> loadedEntries;
		loader.next(100, loadedEntries);

		// Assert:
		EXPECT_TRUE(loadedEntries.empty());
	}

	// endregion

	// region malformed stream

	namespace {
		void AssertCannotLoadMalformedStream(const consumer<std::vector<uint8_t>&>& malformBuffer) {
			// Arrange:
			auto seed = GenerateRandomEntries(7);
			auto buffer = CopyEntriesToStreamBuffer(seed);
			malformBuffer(buffer);

			mocks::MockMemoryStream stream("", buffer);
			ChunkedDataLoader<TestEntryLoaderTraits> loader(stream);

			// Sanity:
			EXPECT_TRUE(loader.hasNext());

			// Act + Assert: attempt to load all entries
			std::vector<TestEntry> loadedEntries;
			EXPECT_THROW(loader.next(8, loadedEntries), catapult_file_io_error);
		}
	}

	TEST(TEST_CLASS, CannotLoadFromStreamWithInsufficientEntries) {
		// Assert: indicate the stream contains more entries than it really does
		AssertCannotLoadMalformedStream([](auto& buffer) { ++reinterpret_cast<uint64_t&>(*buffer.data()); });
	}

	TEST(TEST_CLASS, CannotLoadFromStreamWithTruncatedEntries) {
		// Arrange: corrupt the stream by dropping a byte
		AssertCannotLoadMalformedStream([](auto& buffer) { buffer.pop_back(); });
	}

	// endregion

	// region stateful loader

	namespace {
		struct TestEntryStatefulLoaderTraits {
			using DestinationType = std::vector<size_t>;
			using LoadStateType = size_t;

			static void LoadInto(io::InputStream& input, DestinationType& destination, size_t& state) {
				// Act: add a value derived from both the input and the state
				TestEntry entry;
				input.read({ reinterpret_cast<uint8_t*>(&entry), sizeof(TestEntry) });
				destination.push_back(entry.Beta * ++state);
			}
		};
	}

	TEST(TEST_CLASS, StatefulLoaderPersistsLoadStateAcrossAllLoads) {
		// Arrange:
		constexpr auto Num_Entries = 7u;
		auto seed = GenerateRandomEntries(Num_Entries);
		auto buffer = CopyEntriesToStreamBuffer(seed);
		mocks::MockMemoryStream stream("", buffer);
		ChunkedDataLoader<TestEntryStatefulLoaderTraits> loader(stream);

		// Act: load all values
		std::vector<size_t> loadedValues;
		for (auto count : { 2u, 3u, 2u }) {
			// Sanity:
			EXPECT_TRUE(loader.hasNext());

			// Act:
			loader.next(count, loadedValues);
		}

		EXPECT_FALSE(loader.hasNext());

		// Assert:
		ASSERT_EQ(Num_Entries, loadedValues.size());
		for (auto i = 0u; i < Num_Entries; ++i)
			EXPECT_EQ(seed[i].Beta * (i + 1), loadedValues[i]) << "entry at " << i;
	}

	// endregion
}}
