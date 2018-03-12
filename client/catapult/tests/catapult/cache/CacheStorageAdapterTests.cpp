#include "catapult/cache/CacheStorageAdapter.h"
#include "catapult/cache/SubCachePluginAdapter.h"
#include "tests/test/cache/SimpleCache.h"
#include "tests/test/core/mocks/MockMemoryStream.h"
#include "tests/TestHarness.h"

namespace catapult { namespace cache {

#define TEST_CLASS CacheStorageAdapterTests

	namespace {
#pragma pack(push, 1)

		struct TestEntry {
		public:
			uint64_t Alpha;
			uint8_t Beta;
			uint32_t Gamma;

		public:
			bool operator==(const TestEntry& rhs) const {
				return Alpha == rhs.Alpha && Beta == rhs.Beta && Gamma == rhs.Gamma;
			}
		};

#pragma pack(pop)

		struct TestEntryLoaderTraits {
			using DestinationType = std::vector<TestEntry>;

			static void Load(io::InputStream& input, DestinationType& destination) {
				TestEntry entry;
				input.read({ reinterpret_cast<uint8_t*>(&entry), sizeof(TestEntry) });
				destination.push_back(entry);
			}
		};

		struct TestEntrySaverTraits {
			using SourceType = std::vector<TestEntry>;

			static void Save(const TestEntry& entry, io::OutputStream& output) {
				output.write({ reinterpret_cast<const uint8_t*>(&entry), sizeof(TestEntry) });
			}
		};

		std::vector<TestEntry> GenerateRandomEntries(size_t count) {
			std::vector<TestEntry> entries(count);
			test::FillWithRandomData({ reinterpret_cast<uint8_t*>(entries.data()), entries.size() * sizeof(TestEntry) });
			return entries;
		}

		std::vector<uint8_t> CopyEntriesToStreamBuffer(const std::vector<TestEntry>& entries) {
			auto numHeaderBytes = sizeof(uint64_t);
			uint64_t numBytes = numHeaderBytes + entries.size() * sizeof(TestEntry);
			std::vector<uint8_t> buffer(numBytes);
			reinterpret_cast<uint64_t&>(*buffer.data()) = entries.size();
			memcpy(buffer.data() + numHeaderBytes, entries.data(), numBytes - numHeaderBytes);
			return buffer;
		}

		void AssertAreEqual(const std::vector<TestEntry>& entries, const std::vector<uint8_t>& buffer) {
			// Assert:
			ASSERT_EQ(sizeof(uint64_t) + entries.size() * sizeof(TestEntry), buffer.size());
			EXPECT_EQ(entries.size(), reinterpret_cast<const uint64_t&>(*buffer.data()));
			EXPECT_TRUE(0 == memcmp(entries.data(), buffer.data() + sizeof(uint64_t), entries.size() * sizeof(TestEntry)));
		}
	}

	// region SaveAllData

	TEST(TEST_CLASS, CanSaveAllData_Empty) {
		// Arrange:
		std::vector<uint8_t> buffer;
		mocks::MockMemoryStream stream("", buffer);

		// Act:
		SaveAllData<TestEntrySaverTraits>({}, stream);

		// Assert:
		ASSERT_EQ(sizeof(uint64_t), buffer.size());
		EXPECT_EQ(0u, reinterpret_cast<uint64_t&>(*buffer.data()));
		EXPECT_EQ(1u, stream.numFlushes());
	}

	TEST(TEST_CLASS, CanSaveAllData_NonEmpty) {
		// Arrange:
		std::vector<uint8_t> buffer;
		mocks::MockMemoryStream stream("", buffer);
		auto seed = GenerateRandomEntries(7);

		// Act:
		SaveAllData<TestEntrySaverTraits>(seed, stream);

		// Assert:
		AssertAreEqual(seed, buffer);
		EXPECT_EQ(1u, stream.numFlushes());
	}

	// endregion

	// region ChunkedDataLoader - valid stream

	TEST(TEST_CLASS, CanLoadStorageFromEmptyStream) {
		// Arrange:
		auto buffer = CopyEntriesToStreamBuffer({});
		mocks::MockMemoryStream stream("", buffer);
		auto catapultCache = cache::CatapultCache({});
		ChunkedDataLoader<TestEntryLoaderTraits> loader(stream, catapultCache);

		// Assert:
		EXPECT_FALSE(loader.hasNext());
	}

	namespace {
		void AssertCanLoadAllInOneShot(size_t numEntries, size_t numRequestedEntries) {
			// Arrange:
			auto seed = GenerateRandomEntries(numEntries);
			auto buffer = CopyEntriesToStreamBuffer(seed);
			mocks::MockMemoryStream stream("", buffer);
			auto catapultCache = cache::CatapultCache({});
			ChunkedDataLoader<TestEntryLoaderTraits> loader(stream, catapultCache);

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
		// Assert: requesting 100 entries when only 7 are available, should return all 7
		AssertCanLoadAllInOneShot(7, 100);
	}

	TEST(TEST_CLASS, CanLoadStorageFromNonEmptyStreamInMultipleBatches) {
		// Arrange:
		auto seed = GenerateRandomEntries(7);
		auto buffer = CopyEntriesToStreamBuffer(seed);
		mocks::MockMemoryStream stream("", buffer);
		auto catapultCache = cache::CatapultCache({});
		ChunkedDataLoader<TestEntryLoaderTraits> loader(stream, catapultCache);

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
		auto catapultCache = cache::CatapultCache({});
		ChunkedDataLoader<TestEntryLoaderTraits> loader(stream, catapultCache);

		// Sanity:
		EXPECT_FALSE(loader.hasNext());

		// Act:
		std::vector<TestEntry> loadedEntries;
		loader.next(100, loadedEntries);

		// Assert:
		EXPECT_TRUE(loadedEntries.empty());
	}

	// endregion

	// region ChunkedDataLoader - malformed stream

	namespace {
		void AssertCannotLoadMalformedStream(const consumer<std::vector<uint8_t>&>& malformBuffer) {
			// Arrange:
			auto seed = GenerateRandomEntries(7);
			auto buffer = CopyEntriesToStreamBuffer(seed);
			malformBuffer(buffer);

			mocks::MockMemoryStream stream("", buffer);
			auto catapultCache = cache::CatapultCache({});
			ChunkedDataLoader<TestEntryLoaderTraits> loader(stream, catapultCache);

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

	// region ChunkedDataLoader - stateful loader

	namespace {
		struct TestEntryStatefulLoaderTraits {
			using DestinationType = std::vector<size_t>;
			using LoadStateType = size_t;

			static void Load(io::InputStream& input, DestinationType& destination, size_t& state) {
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
		auto catapultCache = cache::CatapultCache({});
		ChunkedDataLoader<TestEntryStatefulLoaderTraits> loader(stream, catapultCache);

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

	// region ChunkedDataLoader - cache dependent loader

	namespace {
		struct TestEntryCacheDependentLoaderTraits {
			using DestinationType = std::vector<size_t>;
			using DependencyCacheType = test::SimpleCacheT<10>; // this causes the loader to depend on SimpleCacheT<10>

			static void Load(
					io::InputStream& input,
					DestinationType& destination,
					DependencyCacheType::CacheViewType& dependencyCacheType) {
				// Act: add a value derived from both the input and the dependency cache
				TestEntry entry;
				input.read({ reinterpret_cast<uint8_t*>(&entry), sizeof(TestEntry) });
				destination.push_back(entry.Beta + dependencyCacheType.size());
			}
		};

		auto CreateCatapultCacheContainingDependencyCacheWithSize(size_t size) {
			using DependencyCacheType = TestEntryCacheDependentLoaderTraits::DependencyCacheType;
			using SimpleCachePluginAdapter = SubCachePluginAdapter<DependencyCacheType, test::SimpleCacheStorageTraits>;

			// Arrange: create a dependency cache of desired size
			auto pDependencyCache = std::make_unique<DependencyCacheType>();
			{
				auto dependencyCacheDelta = pDependencyCache->createDelta();
				for (auto i = 0u; i < size; ++i)
					dependencyCacheDelta->increment();

				pDependencyCache->commit();
			}

			// - wrap a catapult cache around it
			std::vector<std::unique_ptr<cache::SubCachePlugin>> subCaches(DependencyCacheType::Id + 1);
			subCaches[DependencyCacheType::Id] = std::make_unique<SimpleCachePluginAdapter>(std::move(pDependencyCache));
			return cache::CatapultCache(std::move(subCaches));
		}
	}

	TEST(TEST_CLASS, CacheDependentLoaderProvidesDependencyCacheViewToAllLoads) {
		// Arrange:
		constexpr auto Num_Entries = 7u;
		constexpr auto Dependency_Cache_Size = 4u;
		auto seed = GenerateRandomEntries(Num_Entries);
		auto buffer = CopyEntriesToStreamBuffer(seed);
		mocks::MockMemoryStream stream("", buffer);

		// - create a catapult cache containing a dependency cache and seed it with some value
		auto catapultCache = CreateCatapultCacheContainingDependencyCacheWithSize(Dependency_Cache_Size);
		ChunkedDataLoader<TestEntryCacheDependentLoaderTraits> loader(stream, catapultCache);

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
			EXPECT_EQ(seed[i].Beta + Dependency_Cache_Size, loadedValues[i]) << "entry at " << i;
	}

	// endregion

	// region CacheStorageAdapter

	namespace {
		struct TestEntryStorageTraits : public TestEntryLoaderTraits, public TestEntrySaverTraits
		{};

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
			const std::vector<TestEntry>* createView() const {
				++m_counts.NumCreateViewCalls;
				return &m_entries;
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
	}

	TEST(TEST_CLASS, CanGetNameFromStorageAdapter) {
		// Arrange:
		std::vector<TestEntry> seed;
		VectorToCacheAdapter cache(seed);
		auto catapultCache = cache::CatapultCache({});
		CacheStorageAdapter<VectorToCacheAdapter, TestEntryStorageTraits> storage(cache, catapultCache);

		// Act:
		const auto& name = storage.name();

		// Assert:
		EXPECT_EQ("TestEntry Cache!", name);
	}

	TEST(TEST_CLASS, CanSaveViaCacheStorageAdapter) {
		// Arrange:
		auto seed = GenerateRandomEntries(8);
		VectorToCacheAdapter cache(seed);
		auto catapultCache = cache::CatapultCache({});
		CacheStorageAdapter<VectorToCacheAdapter, TestEntryStorageTraits> storage(cache, catapultCache);

		std::vector<uint8_t> buffer;
		mocks::MockMemoryStream stream("", buffer);

		// Act:
		storage.saveAll(stream);

		// Assert:
		EXPECT_EQ(1u, cache.counts().NumCreateViewCalls);
		EXPECT_EQ(0u, cache.counts().NumCreateDeltaCalls);
		EXPECT_EQ(0u, cache.counts().NumCommitCalls);

		AssertAreEqual(seed, buffer);
		EXPECT_EQ(1u, stream.numFlushes());
	}

	namespace {
		void AssertCanLoadViaCacheStorageAdapter(size_t numEntries, size_t batchSize, size_t numExpectedBatches) {
			// Arrange:
			std::vector<TestEntry> loadedEntries;
			VectorToCacheAdapter cache(loadedEntries);
			auto catapultCache = cache::CatapultCache({});
			CacheStorageAdapter<VectorToCacheAdapter, TestEntryStorageTraits> storage(cache, catapultCache);

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

	// endregion
}}
