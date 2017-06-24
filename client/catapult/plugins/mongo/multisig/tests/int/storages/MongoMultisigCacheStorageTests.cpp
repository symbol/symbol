#include "src/storages/MongoMultisigCacheStorage.h"
#include "plugins/mongo/coremongo/src/MongoDbStorage.h"
#include "plugins/mongo/coremongo/src/mappers/MapperUtils.h"
#include "plugins/mongo/multisig/src/mappers/MultisigEntryMapper.h"
#include "plugins/mongo/multisig/tests/test/MapperTestUtils.h"
#include "plugins/txes/multisig/tests/test/MultisigCacheTestUtils.h"
#include "tests/test/cache/CacheTestUtils.h"
#include "tests/test/mongo/MapperTestUtils.h"
#include "tests/test/mongo/MongoTestUtils.h"
#include "tests/TestHarness.h"

#define TEST_CLASS MongoMultisigCacheStorageTests

using namespace bsoncxx::builder::stream;

namespace catapult { namespace mongo { namespace storages {

	namespace {
		namespace test = catapult::test;

		// default multisig entries are:
		// - without cosignatories, without multisig accounts
		// - with cosignatories, without multisig accounts
		// - without cosignatories, with multisig accounts
		// - with cosignatories, with multisig accounts
		constexpr size_t Num_Default_Entries = 4;
	
		// known keys for multisig entries
		const auto Key1 = test::GenerateRandomData<Key_Size>();
		const auto Key2 = test::GenerateRandomData<Key_Size>();
		const auto Key3 = test::GenerateRandomData<Key_Size>();
		const auto Key4 = test::GenerateRandomData<Key_Size>();

		auto DoNothing = [](auto&, const auto&) {};

		using MultisigEntries = std::vector<state::MultisigEntry>;
		using CacheType = cache::MultisigCache;

		void InsertRandom(utils::KeySet& keys, size_t count) {
			for (auto i = 0u; i < count; ++i)
				keys.insert(test::GenerateRandomData<Key_Size>());
		}

		state::MultisigEntry CreateMultisigEntry(const Key& key, uint8_t numCosignatories, uint8_t numMultisigAccounts) {
			state::MultisigEntry entry(key);
			entry.setMinApproval(12);
			entry.setMinRemoval(23);

			InsertRandom(entry.cosignatories(), numCosignatories);
			InsertRandom(entry.multisigAccounts(), numMultisigAccounts);

			return entry;
		}

		void InsertIntoDelta(cache::MultisigCacheDelta& delta, const MultisigEntries& entries) {
			for (const auto& entry : entries)
				delta.insert(entry);
		}

		void RemoveFromDelta(cache::MultisigCacheDelta& delta, const std::vector<Key>& keysToRemove) {
			for (const auto& key : keysToRemove)
				delta.remove(key);
		}

		void SeedDatabaseAndCache(cache::CatapultCache& cache, plugins::ExternalCacheStorage& storage, const MultisigEntries& entries) {
			// - seed cache
			auto delta = cache.createDelta();
			auto& multisigCacheDelta = delta.sub<CacheType>();
			InsertIntoDelta(multisigCacheDelta, entries);

			// - seed database
			storage.saveDelta(delta);

			// - commit
			cache.commit(Height());
		}

		void SeedDefault(cache::CatapultCache& cache, plugins::ExternalCacheStorage& storage) {
			// seed the database and the multisig cache
			// - MultisigEntry: key / numCosignatories / numMultisigAccounts
			std::vector<state::MultisigEntry> entries{
				CreateMultisigEntry(Key1, 0, 0), // without cosignatories, without multisig accounts
				CreateMultisigEntry(Key2, 5, 0), // with cosignatories, without multisig accounts
				CreateMultisigEntry(Key3, 0, 5), // without cosignatories, with multisig accounts
				CreateMultisigEntry(Key4, 4, 5), // with cosignatories, with multisig accounts
			};

			SeedDatabaseAndCache(cache, storage, entries);
		}

		int64_t NumMultisigEntriesInCollection() {
			auto connection = test::CreateDbConnection();
			auto multisigs = connection[test::DatabaseName()]["multisigs"];
			return multisigs.count({});
		}

		enum class SeedOptions { None, Default };

		class TestContext final : public test::PrepareDatabaseMixin {
		public:
			explicit TestContext(SeedOptions seedOptions)
					: PrepareDatabaseMixin(test::DatabaseName())
					, m_pConfig(test::CreateDefaultMongoStorageConfiguration(test::DatabaseName()))
					, m_pCacheStorage(CreateMongoMultisigCacheStorage(m_pConfig->createDatabaseConnection(), m_pConfig->bulkWriter()))
					, m_cache(test::MultisigCacheFactory::Create()) {
				if (SeedOptions::Default == seedOptions)
					seedDefaultEntries();
			}

		public:
			plugins::ExternalCacheStorage& cacheStorage() {
				return *m_pCacheStorage;
			}

			cache::CatapultCache& cache() {
				return m_cache;
			}

		private:
			void seedDefaultEntries() {
				SeedDefault(m_cache, *m_pCacheStorage);

				// Sanity:
				EXPECT_EQ(Num_Default_Entries, NumMultisigEntriesInCollection());
			}

		private:
			std::shared_ptr<plugins::MongoStorageConfiguration> m_pConfig;
			std::unique_ptr<plugins::ExternalCacheStorage> m_pCacheStorage;
			cache::CatapultCache m_cache;
		};

		bsoncxx::document::value CreateMultisigFilter(const Key& key) {
			return document{}
					<< "multisig.account" << mappers::ToBinary(key)
					<< finalize;
		}

		void AssertMatch(const bsoncxx::document::value& filter, const state::MultisigEntry& entry) {
			auto connection = test::CreateDbConnection();
			auto database = connection[test::DatabaseName()];
			auto dbEntry = database["multisigs"].find_one(filter.view());
			ASSERT_TRUE(dbEntry.is_initialized());

			auto view = dbEntry.get().view();
			EXPECT_EQ(2u, test::GetFieldCount(view));

			auto multisigView = view["multisig"].get_document().view();
			EXPECT_EQ(5u, test::GetFieldCount(multisigView));
			mongo::test::AssertEqualMultisigData(entry, multisigView);
		}

		void AssertDbMultisigEntries(const MultisigEntries& entries) {
			for (const auto& entry : entries) {
				auto filter = CreateMultisigFilter(entry.key());
				AssertMatch(filter, entry);
			}
		}

		void AssertCanSaveDelta(
				const std::vector<state::MultisigEntry>& entries,
				const std::vector<Key>& keysToRemove,
				size_t expectedCollectionSize,
				std::function<void (cache::MultisigCacheDelta&, const MultisigEntries&)> insertAction,
				std::function<void (cache::MultisigCacheDelta&, const std::vector<Key>&)> removeAction,
				SeedOptions seedOptions = SeedOptions::None) {
			// Arrange:
			TestContext context(seedOptions);

			// - prepare cache delta
			auto& cache = context.cache();
			auto delta = cache.createDelta();
			auto& multisigCacheDelta = delta.sub<CacheType>();
			insertAction(multisigCacheDelta, entries);
			removeAction(multisigCacheDelta, keysToRemove);

			// Act:
			context.cacheStorage().saveDelta(delta);
			cache.commit(Height(0));

			// Assert:
			auto expectedEntries = test::ExtractValuesFromCache<CacheType, state::MultisigEntry>(cache.sub<CacheType>());
			EXPECT_EQ(expectedCollectionSize, NumMultisigEntriesInCollection());
			EXPECT_EQ(expectedCollectionSize, expectedEntries.size());
			AssertDbMultisigEntries(expectedEntries);
		}
	}

	// region saveDelta

	TEST(TEST_CLASS, SaveDeltaIsNoOpIfCacheIsClean) {
		// Assert:
		AssertCanSaveDelta({}, {}, 0, DoNothing, DoNothing);
	}

	// region added entries

	TEST(TEST_CLASS, CanSaveSingleMultisigEntry_New) {
		// Arrange:
		// - MultisigEntry: key / numCosignatories / numMultisigAccounts
		MultisigEntries entries{ CreateMultisigEntry(test::GenerateRandomData<Key_Size>(), 4, 5) };

		// Assert:
		AssertCanSaveDelta(entries, {}, 1, InsertIntoDelta, DoNothing);
	}

	TEST(TEST_CLASS, CanSaveMultipleMultisigEntries_New) {
		// Arrange:
		// - MultisigEntry: key / numCosignatories / numMultisigAccounts
		MultisigEntries entries{
			CreateMultisigEntry(test::GenerateRandomData<Key_Size>(), 4, 5),
			CreateMultisigEntry(test::GenerateRandomData<Key_Size>(), 0, 5),
			CreateMultisigEntry(test::GenerateRandomData<Key_Size>(), 4, 0)
		};

		// Assert:
		AssertCanSaveDelta(entries, {}, 3, InsertIntoDelta, DoNothing);
	}

	// endregion

	// region modified entries

	TEST(TEST_CLASS, CanSaveSingleMultisigEntry_ModifiedByAdding) {
		// Arrange:
		// - MultisigEntry: key / numCosignatories / numMultisigAccounts
		// - note that multisig entry with key key1 already exists in the multisig cache
		MultisigEntries entries{ CreateMultisigEntry(Key1, 4, 5) };

		// Assert:
		AssertCanSaveDelta(entries, {}, Num_Default_Entries, InsertIntoDelta, DoNothing, SeedOptions::Default);
	}

	TEST(TEST_CLASS, CanSaveMultipleMultisigEntries_ModifiedByAdding) {
		// Arrange:
		// - MultisigEntry: key / numCosignatories / numMultisigAccounts
		// - note that multisig entries with keys key1 - key4 already exists in the multisig cache
		MultisigEntries entries{
			CreateMultisigEntry(Key1, 4, 5),
			CreateMultisigEntry(Key2, 5, 0),
			CreateMultisigEntry(Key3, 3, 3),
			CreateMultisigEntry(Key4, 0, 7)
		};

		// Assert:
		AssertCanSaveDelta(entries, {}, Num_Default_Entries, InsertIntoDelta, DoNothing, SeedOptions::Default);
	}

	// endregion

	// region removing multisig entries

	TEST(TEST_CLASS, CanSaveSingleMultisigEntry_RemoveEntry) {
		// Assert:
		// - note that multisig entry with key key1 exists in the multisig cache
		AssertCanSaveDelta({}, { Key1 }, Num_Default_Entries - 1, DoNothing, RemoveFromDelta, SeedOptions::Default);
	}

	TEST(TEST_CLASS, CanSaveMultipleMultisigEntries_RemoveEntries) {
		// Assert:
		// - note that multisig entries with keys key1, key3 and key4 exists in the multisig cache
		AssertCanSaveDelta({}, { Key1, Key3, Key4 }, Num_Default_Entries - 3, DoNothing, RemoveFromDelta, SeedOptions::Default);
	}

	// endregion

	// region mixed operations

	TEST(TEST_CLASS, CanSaveMultipleMultisigEntries_Added_Modified_Removed) {
		// Arrange:
		// - MultisigEntry: key / numCosignatories / numMultisigAccounts
		// - note that multisig entries with keys key2, key3 and key4 exists in the multisig cache
		MultisigEntries entries{
			CreateMultisigEntry(test::GenerateRandomData<Key_Size>(), 4, 5), // add
			CreateMultisigEntry(Key2, 7, 2), // modify
		};

		// Assert:
		AssertCanSaveDelta(
				entries,
				{ Key3, Key4 }, // remove
				Num_Default_Entries + 1 + 0 - 2, // changes due to add / modify / remove
				InsertIntoDelta,
				RemoveFromDelta,
				SeedOptions::Default);
	}

	// endregion

	// endregion

	// region loadAll

	namespace {
		void SeedDatabase(const MultisigEntries& entries) {
			auto connection = test::CreateDbConnection();
			auto multisigs = connection[test::DatabaseName()]["multisigs"];
			for (const auto& entry : entries) {
				auto dbEntry = mappers::ToDbModel(entry);
				multisigs.insert_one(dbEntry.view());
			}
		}

		void AssertCacheEntry(const state::MultisigEntry& expectedEntry, const state::MultisigEntry& entry) {
			EXPECT_EQ(expectedEntry.key(), entry.key());
			EXPECT_EQ(expectedEntry.minApproval(), entry.minApproval());
			EXPECT_EQ(expectedEntry.minRemoval(), entry.minRemoval());
			EXPECT_EQ(expectedEntry.cosignatories(), entry.cosignatories());
			EXPECT_EQ(expectedEntry.multisigAccounts(), entry.multisigAccounts());
		}

		void AssertCacheContents(const CacheType& cache, size_t expectedSize) {
			auto connection = test::CreateDbConnection();
			auto multisigs = connection[test::DatabaseName()]["multisigs"];
			size_t count = 0;
			auto view = cache.createView();
			for (auto iter = view->cbegin(); view->cend() != iter; ++iter) {
				const auto& entry = iter->second;

				auto filter = CreateMultisigFilter(entry.key());
				auto document = multisigs.find_one(filter.view());
				ASSERT_TRUE(document.is_initialized());

				auto documentView = document.get().view();
				EXPECT_EQ(2u, test::GetFieldCount(documentView));
				auto expectedEntry = mappers::ToMultisigEntry(documentView);
				AssertCacheEntry(expectedEntry, entry);

				++count;
			}

			EXPECT_EQ(expectedSize, count);
		}

		void AssertCanLoadAll(const MultisigEntries& seedEntities, size_t expectedCacheSize) {
			// Arrange:
			TestContext context(SeedOptions::None);
			SeedDatabase(seedEntities);

			// Sanity;
			EXPECT_EQ(expectedCacheSize, NumMultisigEntriesInCollection());

			// Act:
			context.cacheStorage().loadAll(context.cache(), Height(0));

			// Assert:
			AssertCacheContents(context.cache().sub<CacheType>(), expectedCacheSize);
		}
	}

	TEST(TEST_CLASS, LoadAllIsNoOpIfDbCollectionIsEmpty) {
		// Assert:
		AssertCanLoadAll({}, 0u);
	}

	TEST(TEST_CLASS, CanLoadAll_SingleMultisigEntry) {
		// Arrange:
		// - MultisigEntry: key / numCosignatories / numMultisigAccounts
		MultisigEntries seedEntries{ CreateMultisigEntry(test::GenerateRandomData<Key_Size>(), 4, 5) };

		// Assert:
		AssertCanLoadAll(seedEntries, 1);
	}

	TEST(TEST_CLASS, CanLoadAll_MultipleMultisigEntries) {
		// Arrange:
		// - MultisigEntry: key / numCosignatories / numMultisigAccounts
		MultisigEntries seedEntries{
			CreateMultisigEntry(test::GenerateRandomData<Key_Size>(), 4, 5),
			CreateMultisigEntry(test::GenerateRandomData<Key_Size>(), 6, 0),
			CreateMultisigEntry(test::GenerateRandomData<Key_Size>(), 0, 7),
			CreateMultisigEntry(test::GenerateRandomData<Key_Size>(), 0, 0),
			CreateMultisigEntry(test::GenerateRandomData<Key_Size>(), 123, 234)
		};

		// Assert:
		AssertCanLoadAll(seedEntries, 5);
	}

	// endregion
}}}
