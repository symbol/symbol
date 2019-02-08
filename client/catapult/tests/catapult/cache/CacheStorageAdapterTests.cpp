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

#include "catapult/cache/CacheStorageAdapter.h"
#include "catapult/cache/SubCachePluginAdapter.h"
#include "tests/catapult/cache/test/CacheSerializationTestUtils.h"
#include "tests/test/cache/SimpleCache.h"
#include "tests/test/core/mocks/MockMemoryStream.h"
#include "tests/TestHarness.h"

namespace catapult { namespace cache {

#define TEST_CLASS CacheStorageAdapterTests

	namespace {
		using TestEntry = test::CacheSerializationTestEntry;
		using TestEntryLoaderTraits = test::CacheSerializationTestEntryLoaderTraits;

		constexpr auto GenerateRandomEntries = test::GenerateRandomCacheSerializationTestEntries;
		constexpr auto CopyEntriesToStreamBuffer = test::CopyCacheSerializationTestEntriesToStreamBuffer;

		void AssertAreEqual(const std::vector<TestEntry>& entries, const std::vector<uint8_t>& buffer) {
			// Assert:
			ASSERT_EQ(sizeof(uint64_t) + entries.size() * sizeof(TestEntry), buffer.size());
			EXPECT_EQ(entries.size(), reinterpret_cast<const uint64_t&>(*buffer.data()));
			EXPECT_EQ_MEMORY(entries.data(), buffer.data() + sizeof(uint64_t), entries.size() * sizeof(TestEntry));
		}

		// region VectorToCacheAdapter

		class ViewAdapter {
		public:
			explicit ViewAdapter(const std::vector<TestEntry>& entries) : m_entries(entries)
			{}

		public:
			size_t size() const {
				return m_entries.size();
			}

			const std::vector<TestEntry>* tryMakeIterableView() const {
				return &m_entries;
			}

		private:
			const std::vector<TestEntry>& m_entries;
		};

		class VectorToCacheAdapter {
		public:
			static constexpr auto Name = "TestEntry Cache!";

		public:
			explicit VectorToCacheAdapter(std::vector<TestEntry>& entries) : m_entries(entries)
			{}

		public:
			struct CallCounts {
				size_t NumCreateViewCalls = 0;
				size_t NumCreateDeltaCalls = 0;
				size_t NumCommitCalls = 0;
			};

		public:
			const CallCounts& counts() const {
				return m_counts;
			}

		public:
			std::unique_ptr<ViewAdapter> createView() const {
				++m_counts.NumCreateViewCalls;
				return std::make_unique<ViewAdapter>(m_entries);
			}

			std::vector<TestEntry>* createDelta() {
				++m_counts.NumCreateDeltaCalls;
				return &m_entries;
			}

			void commit() {
				++m_counts.NumCommitCalls;
			}

		private:
			std::vector<TestEntry>& m_entries;
			mutable CallCounts m_counts;
		};

		// endregion

		struct TestEntryStorageTraits : public TestEntryLoaderTraits {
			using SourceType = ViewAdapter;

			static void Save(const TestEntry& entry, io::OutputStream& output) {
				output.write({ reinterpret_cast<const uint8_t*>(&entry), sizeof(TestEntry) });
			}
		};
	}

	TEST(TEST_CLASS, CanGetNameFromStorageAdapter) {
		// Arrange:
		std::vector<TestEntry> seed;
		VectorToCacheAdapter cache(seed);
		CacheStorageAdapter<VectorToCacheAdapter, TestEntryStorageTraits> storage(cache);

		// Act:
		const auto& name = storage.name();

		// Assert:
		EXPECT_EQ("TestEntry Cache!", name);
	}

	namespace {
		void AssertCanSaveViaCacheStorageAdapter(uint64_t numEntries) {
			// Arrange:
			auto seed = GenerateRandomEntries(numEntries);
			VectorToCacheAdapter cache(seed);
			CacheStorageAdapter<VectorToCacheAdapter, TestEntryStorageTraits> storage(cache);

			std::vector<uint8_t> buffer;
			mocks::MockMemoryStream stream("", buffer);

			// Act:
			storage.saveAll(stream);

			// Assert:
			EXPECT_EQ(1u, cache.counts().NumCreateViewCalls);
			EXPECT_EQ(0u, cache.counts().NumCreateDeltaCalls);
			EXPECT_EQ(0u, cache.counts().NumCommitCalls);

			AssertAreEqual(seed, buffer);
			EXPECT_EQ(numEntries, reinterpret_cast<uint64_t&>(*buffer.data()));
			EXPECT_EQ(1u, stream.numFlushes());
		}
	}

	TEST(TEST_CLASS, CanSaveEmptyDataViaCacheStorageAdapter) {
		// Assert:
		AssertCanSaveViaCacheStorageAdapter(0);
	}

	TEST(TEST_CLASS, CanSaveNonEmptyDataViaCacheStorageAdapter) {
		// Assert:
		AssertCanSaveViaCacheStorageAdapter(8);
	}

	namespace {
		void AssertCanLoadViaCacheStorageAdapter(size_t numEntries, size_t batchSize, size_t numExpectedBatches) {
			// Arrange:
			std::vector<TestEntry> loadedEntries;
			VectorToCacheAdapter cache(loadedEntries);
			CacheStorageAdapter<VectorToCacheAdapter, TestEntryStorageTraits> storage(cache);

			auto seed = GenerateRandomEntries(numEntries);
			auto buffer = CopyEntriesToStreamBuffer(seed);
			mocks::MockMemoryStream stream("", buffer);

			// Act:
			storage.loadAll(stream, batchSize);

			// Assert:
			EXPECT_EQ(0u, cache.counts().NumCreateViewCalls);
			EXPECT_EQ(1u, cache.counts().NumCreateDeltaCalls);
			EXPECT_EQ(numExpectedBatches, cache.counts().NumCommitCalls);

			EXPECT_EQ(seed, loadedEntries);
			EXPECT_EQ(0u, stream.numFlushes());
		}
	}

	TEST(TEST_CLASS, CanLoadViaCacheStorageAdapter_SingleBatch) {
		// Assert:
		AssertCanLoadViaCacheStorageAdapter(7, 100, 1);
	}

	TEST(TEST_CLASS, CanLoadViaCacheStorageAdapter_MultipleBatches) {
		// Assert:
		AssertCanLoadViaCacheStorageAdapter(7, 2, 4);
	}
}}
