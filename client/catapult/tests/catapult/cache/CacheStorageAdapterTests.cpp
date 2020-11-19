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

#include "catapult/cache/CacheStorageAdapter.h"
#include "catapult/cache/CatapultCache.h"
#include "catapult/cache/SubCachePluginAdapter.h"
#include "tests/catapult/cache/test/CacheSerializationTestUtils.h"
#include "tests/test/cache/SimpleCache.h"
#include "tests/test/cache/UnsupportedSubCachePlugin.h"
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
			EXPECT_EQ(entries.size(), reinterpret_cast<const uint64_t&>(buffer[0]));
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
			using CacheViewType = ViewAdapter;

		public:
			static constexpr size_t Id = 0;
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
			CacheViewType createView() const {
				++m_counts.NumCreateViewCalls;
				return CacheViewType(m_entries);
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

		// region TestEntryStorageTraits

		struct TestEntryStorageTraits : public TestEntryLoaderTraits {
			using SourceType = ViewAdapter;

			static void Save(const TestEntry& entry, io::OutputStream& output) {
				output.write({ reinterpret_cast<const uint8_t*>(&entry), sizeof(TestEntry) });
			}
		};

		// endregion

		// region VectorToCacheAdapterSubCachePlugin

		class VectorToCacheAdapterSubCachePlugin : public test::UnsupportedSubCachePlugin<VectorToCacheAdapter> {
		public:
			explicit VectorToCacheAdapterSubCachePlugin(std::vector<TestEntry>& entries) : m_cache(entries)
			{}

		public:
			auto& cache() {
				return m_cache;
			}

		public:
			std::unique_ptr<const SubCacheView> createView() const override {
				return std::make_unique<VectorToCacheAdapterSubCacheView>(m_cache.createView());
			}

		private:
			class VectorToCacheAdapterSubCacheView : public test::UnsupportedSubCacheView {
			public:
				explicit VectorToCacheAdapterSubCacheView(const ViewAdapter& view) : m_view(view)
				{}

			public:
				const void* get() const override {
					return &m_view;
				}

			private:
				ViewAdapter m_view;
			};

		private:
			VectorToCacheAdapter m_cache;
		};

		// endregion
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
		void AssertCanSaveAllViaCacheStorageAdapter(uint64_t numEntries) {
			// Arrange:
			auto seed = GenerateRandomEntries(numEntries);
			auto pPlugin = std::make_unique<VectorToCacheAdapterSubCachePlugin>(seed);
			CacheStorageAdapter<VectorToCacheAdapter, TestEntryStorageTraits> storage(pPlugin->cache());
			const auto& typedCache = pPlugin->cache();

			std::vector<std::unique_ptr<SubCachePlugin>> subCaches;
			subCaches.push_back(std::move(pPlugin));
			CatapultCache catapultCache(std::move(subCaches));
			auto cacheView = catapultCache.createView();

			std::vector<uint8_t> buffer;
			mocks::MockMemoryStream stream(buffer);

			// Act:
			storage.saveAll(cacheView, stream);

			// Assert:
			EXPECT_EQ(1u, typedCache.counts().NumCreateViewCalls);
			EXPECT_EQ(0u, typedCache.counts().NumCreateDeltaCalls);
			EXPECT_EQ(0u, typedCache.counts().NumCommitCalls);

			AssertAreEqual(seed, buffer);
			EXPECT_EQ(numEntries, reinterpret_cast<uint64_t&>(buffer[0]));
			EXPECT_EQ(1u, stream.numFlushes());
		}
	}

	TEST(TEST_CLASS, CanSaveAllEmptyDataViaCacheStorageAdapter) {
		AssertCanSaveAllViaCacheStorageAdapter(0);
	}

	TEST(TEST_CLASS, CanSaveAllDataViaCacheStorageAdapter) {
		AssertCanSaveAllViaCacheStorageAdapter(8);
	}

	TEST(TEST_CLASS, CannotSaveSummaryDataViaCacheStorageAdapter) {
		// Arrange:
		auto seed = GenerateRandomEntries(3);
		VectorToCacheAdapter cache(seed);
		CacheStorageAdapter<VectorToCacheAdapter, TestEntryStorageTraits> storage(cache);

		CatapultCache catapultCache({});
		auto cacheDelta = catapultCache.createDelta();

		std::vector<uint8_t> buffer;
		mocks::MockMemoryStream stream(buffer);

		// Act + Assert:
		EXPECT_THROW(storage.saveSummary(cacheDelta, stream), catapult_invalid_argument);
	}

	namespace {
		void AssertCanLoadViaCacheStorageAdapter(size_t numEntries, size_t batchSize, size_t numExpectedBatches) {
			// Arrange:
			std::vector<TestEntry> loadedEntries;
			VectorToCacheAdapter cache(loadedEntries);
			CacheStorageAdapter<VectorToCacheAdapter, TestEntryStorageTraits> storage(cache);

			auto seed = GenerateRandomEntries(numEntries);
			auto buffer = CopyEntriesToStreamBuffer(seed);
			mocks::MockMemoryStream stream(buffer);

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
		AssertCanLoadViaCacheStorageAdapter(7, 100, 1);
	}

	TEST(TEST_CLASS, CanLoadViaCacheStorageAdapter_MultipleBatches) {
		AssertCanLoadViaCacheStorageAdapter(7, 2, 4);
	}
}}
