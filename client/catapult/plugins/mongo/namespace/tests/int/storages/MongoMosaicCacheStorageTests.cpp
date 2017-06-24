#include "src/storages/MongoMosaicCacheStorage.h"
#include "src/storages/CacheStorageUtils.h"
#include "plugins/mongo/coremongo/src/MongoDbStorage.h"
#include "plugins/mongo/coremongo/src/mappers/MapperUtils.h"
#include "plugins/mongo/namespace/src/mappers/MosaicDescriptorMapper.h"
#include "plugins/mongo/namespace/tests/test/MapperTestUtils.h"
#include "plugins/txes/namespace/src/state/MosaicDescriptor.h"
#include "plugins/txes/namespace/src/state/MosaicEntry.h"
#include "plugins/txes/namespace/tests/test/MosaicCacheTestUtils.h"
#include "plugins/txes/namespace/tests/test/MosaicTestUtils.h"
#include "tests/test/mongo/MapperTestUtils.h"
#include "tests/test/mongo/MongoTestUtils.h"
#include "tests/TestHarness.h"

#ifdef _WIN32
#include <boost/config/compiler/visualc.hpp>
#endif

#define TEST_CLASS MongoMosaicCacheStorageTests

using namespace bsoncxx::builder::stream;

namespace catapult { namespace mongo { namespace storages {

	namespace {
		namespace test = catapult::test;

		// default mosaic histories are:
		// - namespace id 100, mosaic id 200 with depth one
		// - namespace id 101, mosaic id 201 with depth two
		// - namespace id 102, mosaic id 202 with depth three
		constexpr size_t Num_Default_Entries = 6;

		auto DoNothing = [](auto&, const auto&) {};

		using Descriptors = std::vector<state::MosaicDescriptor>;

		Descriptors ExtractDescriptorsFromCache(const cache::MosaicCache& cache) {
			Descriptors allDescriptors;
			auto view = cache.createView();
			for (auto iter = view->cbegin(); view->cend() != iter; ++iter) {
				auto descriptors = MosaicDescriptorsFromHistory(iter->second);
				std::copy(descriptors.begin(), descriptors.end(), std::back_inserter(allDescriptors));
			}

			return allDescriptors;
		}

		std::shared_ptr<const state::MosaicEntry> CreateMosaicEntry(NamespaceId::ValueType namespaceId, MosaicId::ValueType mosaicId) {
			return test::CreateMosaicEntry(
					NamespaceId(namespaceId),
					MosaicId(mosaicId),
					Height(345),
					test::GenerateRandomData<Key_Size>(),
					Amount(456),
					ArtifactDuration(567));
		}

		void InsertIntoDelta(cache::MosaicCacheDelta& delta, const Descriptors& descriptors) {
			for (const auto& descriptor : descriptors)
				delta.insert(*descriptor.pEntry);
		}

		void RemoveFromDelta(cache::MosaicCacheDelta& delta, const std::vector<MosaicId>& idsToRemove) {
			for (auto id : idsToRemove)
				delta.remove(id);
		}

		void SeedDatabaseAndCache(cache::CatapultCache& cache, plugins::ExternalCacheStorage& storage, const Descriptors& descriptors) {
			// - seed cache
			auto delta = cache.createDelta();
			auto& mosaicCacheDelta = delta.sub<cache::MosaicCache>();
			InsertIntoDelta(mosaicCacheDelta, descriptors);

			// - seed database
			storage.saveDelta(delta);

			// - commit
			cache.commit(Height());
		}

		void SeedDefault(cache::CatapultCache& cache, plugins::ExternalCacheStorage& storage) {
			// seed the database and the mosaic cache
			// - CreateMosaicEntry: namespaceId / mosaicId
			// - MosaicDescriptor: mosaicEntry / index / isActive
			std::vector<state::MosaicDescriptor> entries{
				state::MosaicDescriptor{ CreateMosaicEntry(100, 200), 0, true }, // history depth one
				state::MosaicDescriptor{ CreateMosaicEntry(101, 201), 0, false }, // history depth two
				state::MosaicDescriptor{ CreateMosaicEntry(101, 201), 1, true },
				state::MosaicDescriptor{ CreateMosaicEntry(102, 202), 0, false }, // history depth three
				state::MosaicDescriptor{ CreateMosaicEntry(102, 202), 1, false },
				state::MosaicDescriptor{ CreateMosaicEntry(102, 202), 2, true }
			};

			SeedDatabaseAndCache(cache, storage, entries);
		}

		int64_t NumMosaicsInCollection() {
			auto connection = test::CreateDbConnection();
			auto mosaics = connection[test::DatabaseName()]["mosaics"];
			return mosaics.count({});
		}

		enum class SeedOptions { None, Default };

		class TestContext final : public test::PrepareDatabaseMixin {
		public:
			explicit TestContext(SeedOptions seedOptions)
					: PrepareDatabaseMixin(test::DatabaseName())
					, m_pConfig(test::CreateDefaultMongoStorageConfiguration(test::DatabaseName()))
					, m_pCacheStorage(CreateMongoMosaicCacheStorage(m_pConfig->createDatabaseConnection(), m_pConfig->bulkWriter()))
					, m_cache(test::MosaicCacheFactory::Create()) {
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
				EXPECT_EQ(Num_Default_Entries, NumMosaicsInCollection());
			}

		private:
			std::shared_ptr<plugins::MongoStorageConfiguration> m_pConfig;
			std::unique_ptr<plugins::ExternalCacheStorage> m_pCacheStorage;
			cache::CatapultCache m_cache;
		};

		bsoncxx::document::value CreateMosaicFilter(NamespaceId namespaceId, MosaicId mosaicId, uint32_t index) {
			return document{}
				<< "$and"
				<< open_array
					<< open_document
						<< "mosaic.namespaceId" << mappers::ToInt64(namespaceId)
					<< close_document
					<< open_document
						<< "mosaic.mosaicId" << mappers::ToInt64(mosaicId)
					<< close_document
					<< open_document
						<< "meta.index" << static_cast<int32_t>(index)
					<< close_document
				<< close_array
				<< finalize;
		}

		void AssertMatch(const bsoncxx::document::value& filter, const state::MosaicDescriptor& descriptor) {
			auto connection = test::CreateDbConnection();
			auto database = connection[test::DatabaseName()];
			auto dbMosaic = database["mosaics"].find_one(filter.view());
			ASSERT_TRUE(dbMosaic.is_initialized());

			auto view = dbMosaic.get().view();
			EXPECT_EQ(3u, test::GetFieldCount(view));

			auto metaView = view["meta"].get_document().view();
			EXPECT_EQ(2u, test::GetFieldCount(metaView));
			mongo::test::AssertEqualMosaicMetadata(descriptor, metaView);

			auto mosaicView = view["mosaic"].get_document().view();
			EXPECT_EQ(7u, test::GetFieldCount(mosaicView));
			mongo::test::AssertEqualMosaicData(descriptor, mosaicView);
		}

		void AssertDbMosaics(const Descriptors& descriptors) {
			for (const auto& descriptor : descriptors) {
				const auto& entry = *descriptor.pEntry;
				auto filter = CreateMosaicFilter(entry.namespaceId(), entry.mosaicId(), descriptor.Index);
				AssertMatch(filter, descriptor);
			}
		}

		void AssertCanSaveDelta(
				const std::vector<state::MosaicDescriptor>& descriptors,
				const std::vector<MosaicId>& idsToRemove,
				size_t expectedCollectionSize,
				std::function<void (cache::MosaicCacheDelta&, const Descriptors&)> insertAction,
				std::function<void (cache::MosaicCacheDelta&, const std::vector<MosaicId>&)> removeAction,
				SeedOptions seedOptions = SeedOptions::None) {
			// Arrange:
			TestContext context(seedOptions);

			// - prepare cache delta
			auto& cache = context.cache();
			auto delta = cache.createDelta();
			auto& mosaicCacheDelta = delta.sub<cache::MosaicCache>();
			insertAction(mosaicCacheDelta, descriptors);
			removeAction(mosaicCacheDelta, idsToRemove);

			// Act:
			context.cacheStorage().saveDelta(delta);
			cache.commit(Height(0));

			// Assert:
			auto expectedDescriptors = ExtractDescriptorsFromCache(cache.sub<cache::MosaicCache>());
			EXPECT_EQ(expectedCollectionSize, NumMosaicsInCollection());
			EXPECT_EQ(expectedCollectionSize, expectedDescriptors.size());
			AssertDbMosaics(expectedDescriptors);
		}
	}

	// region saveDelta

	TEST(TEST_CLASS, SaveDeltaIsNoOpIfCacheIsClean) {
		// Assert:
		AssertCanSaveDelta({}, {}, 0, DoNothing, DoNothing);
	}

	// region added histories

	TEST(TEST_CLASS, CanSaveSingleMosaic_New) {
		// Arrange:
		// - CreateMosaicEntry: namespaceId / mosaicId
		// - MosaicDescriptor: mosaicEntry / index / isActive
		std::vector<state::MosaicDescriptor> entries{ state::MosaicDescriptor{ CreateMosaicEntry(123, 234), 0, true } };

		// Assert:
		AssertCanSaveDelta(entries, {}, 1, InsertIntoDelta, DoNothing);
	}

	TEST(TEST_CLASS, CanSaveMultipleMosaics_New_SameNamespaceId_DifferentMosaicIds) {
		// Arrange:
		// - CreateMosaicEntry: namespaceId / mosaicId
		// - MosaicDescriptor: mosaicEntry / index / isActive
		std::vector<state::MosaicDescriptor> entries{
			state::MosaicDescriptor{ CreateMosaicEntry(123, 234), 0, true },
			state::MosaicDescriptor{ CreateMosaicEntry(123, 235), 0, true },
			state::MosaicDescriptor{ CreateMosaicEntry(123, 236), 0, true }
		};

		// Assert:
		AssertCanSaveDelta(entries, {}, 3, InsertIntoDelta, DoNothing);
	}

	TEST(TEST_CLASS, CanSaveMultipleMosaics_New_SameNamespaceId_SameMosaicId) {
		// Arrange:
		// - CreateMosaicEntry: namespaceId / mosaicId
		// - MosaicDescriptor: mosaicEntry / index / isActive
		std::vector<state::MosaicDescriptor> entries{
			state::MosaicDescriptor{ CreateMosaicEntry(123, 234), 0, false },
			state::MosaicDescriptor{ CreateMosaicEntry(123, 234), 1, false },
			state::MosaicDescriptor{ CreateMosaicEntry(123, 234), 2, true }
		};

		// Assert:
		AssertCanSaveDelta(entries, {}, 3, InsertIntoDelta, DoNothing);
	}

	TEST(TEST_CLASS, CanSaveMultipleMosaics_New_DifferentNamespaceIds) {
		// Arrange:
		// - CreateMosaicEntry: namespaceId / mosaicId
		// - MosaicDescriptor: mosaicEntry / index / isActive
		std::vector<state::MosaicDescriptor> entries{
			state::MosaicDescriptor{ CreateMosaicEntry(123, 234), 0, true },
			state::MosaicDescriptor{ CreateMosaicEntry(124, 235), 0, true },
			state::MosaicDescriptor{ CreateMosaicEntry(125, 236), 0, true }
		};

		// Assert:
		AssertCanSaveDelta(entries, {}, 3, InsertIntoDelta, DoNothing);
	}

	// endregion

	// region modified histories

	// region modifying by adding

	TEST(TEST_CLASS, CanSaveSingleMosaic_ModifiedByAdding) {
		// Arrange:
		// - CreateMosaicEntry: namespaceId / mosaicId
		// - MosaicDescriptor: mosaicEntry / index / isActive
		// - note that mosaic history with namespace id 100 and mosaic id 200 already exists in the mosaic cache
		std::vector<state::MosaicDescriptor> entries{ state::MosaicDescriptor{ CreateMosaicEntry(100, 200), 0, true } };

		// Assert:
		AssertCanSaveDelta(entries, {}, Num_Default_Entries + 1, InsertIntoDelta, DoNothing, SeedOptions::Default);
	}

	TEST(TEST_CLASS, CanSaveMultipleMosaics_ModifiedByAdding) {
		// Arrange:
		// - CreateMosaicEntry: namespaceId / mosaicId
		// - MosaicDescriptor: mosaicEntry / index / isActive
		// - note that mosaic history with namespace id 100 - 102 and mosaic id 200 - 202 already exists in the mosaic cache
		std::vector<state::MosaicDescriptor> entries{
			state::MosaicDescriptor{ CreateMosaicEntry(100, 200), 0, true },
			state::MosaicDescriptor{ CreateMosaicEntry(100, 200), 0, true },
			state::MosaicDescriptor{ CreateMosaicEntry(101, 201), 0, true },
			state::MosaicDescriptor{ CreateMosaicEntry(102, 202), 0, true }
		};

		// Assert:
		AssertCanSaveDelta(entries, {}, Num_Default_Entries + 4, InsertIntoDelta, DoNothing, SeedOptions::Default);
	}

	// endregion

	// region modifying by removing partial history

	TEST(TEST_CLASS, CanSaveSingleMosaic_ModifiedByRemovingPartialHistory) {
		// Assert:
		// - note that mosaic history with mosaic id 201 already exists in the mosaic cache with depth 2
		AssertCanSaveDelta({}, { MosaicId(201) }, Num_Default_Entries - 1, DoNothing, RemoveFromDelta, SeedOptions::Default);
	}

	TEST(TEST_CLASS, CanSaveMultipleMosaics_ModifiedByRemovingPartialHistory) {
		// Assert:
		// - note that mosaic histories with mosaic ids 201 and 202 already exists in the mosaic cache with depth 2 and 3
		AssertCanSaveDelta(
				{},
				{ MosaicId(201), MosaicId(202) },
				Num_Default_Entries - 2,
				DoNothing,
				RemoveFromDelta,
				SeedOptions::Default);
	}

	// endregion

	// endregion

	// region removing full history

	TEST(TEST_CLASS, CanSaveSingleMosaic_RemovingFullHistory) {
		// Assert:
		// - note that mosaic history with mosaic id 200 already exists in the mosaic cache with depth 1
		AssertCanSaveDelta({}, { MosaicId(200) }, Num_Default_Entries - 1, DoNothing, RemoveFromDelta, SeedOptions::Default);
	}

	TEST(TEST_CLASS, CanSaveMultipleMosaics_RemovingFullHistory) {
		// Assert:
		// - note that mosaic history with mosaic id 200 and 201 already exists in the mosaic cache with depth 1 and 2
		AssertCanSaveDelta(
				{},
				{ MosaicId(200), MosaicId(201), MosaicId(201) },
				Num_Default_Entries - 3,
				DoNothing,
				RemoveFromDelta,
				SeedOptions::Default);
	}

	// endregion

	// region mixed operations

	TEST(TEST_CLASS, CanSaveMultipleMosaics_Added_Modified_Removed) {
		// Arrange:
		// - CreateMosaicEntry: namespaceId / mosaicId
		// - MosaicDescriptor: mosaicEntry / index / isActive
		// - note that mosaic history with namespace id 100 and mosaic id 200 already exists in the mosaic cache
		std::vector<state::MosaicDescriptor> entries{
			state::MosaicDescriptor{ CreateMosaicEntry(123, 234), 0, true }, // added
			state::MosaicDescriptor{ CreateMosaicEntry(100, 200), 0, true } // modified by adding
		};

		// Assert:
		// - note that mosaic history with mosaic id 200 - 202 already exists in the mosaic cache with depth 1 - 3
		AssertCanSaveDelta(
				entries,
				{ MosaicId(200), MosaicId(201), MosaicId(201), MosaicId(202) }, // removed / removed / modified by partially removing
				Num_Default_Entries + 2 - 4,
				InsertIntoDelta,
				RemoveFromDelta,
				SeedOptions::Default);
	}

	// endregion

	// endregion

	// region loadAll

	namespace {
		void SeedDatabase(const Descriptors& descriptors) {
			auto connection = test::CreateDbConnection();
			auto mosaics = connection[test::DatabaseName()]["mosaics"];
			for (const auto& descriptor : descriptors) {
				auto dbMosaic = mappers::ToDbModel(descriptor);
				mosaics.insert_one(dbMosaic.view());
			}
		}

		void AssertCacheEntry(const state::MosaicEntry& expectedEntry, const state::MosaicEntry& entry) {
			EXPECT_EQ(expectedEntry.namespaceId(), entry.namespaceId());
			EXPECT_EQ(expectedEntry.mosaicId(), entry.mosaicId());
			EXPECT_EQ(expectedEntry.supply(), entry.supply());
			EXPECT_FALSE(entry.hasLevy());

			const auto& definition = entry.definition();
			const auto& expectedDefinition = expectedEntry.definition();
			EXPECT_EQ(expectedDefinition.height(), definition.height());
			EXPECT_EQ(expectedDefinition.owner(), definition.owner());
			test::AssertMosaicDefinitionProperties(expectedDefinition.properties(), definition.properties());
		}

		void AssertCacheContents(const cache::MosaicCache& cache, size_t expectedSize) {
			auto connection = test::CreateDbConnection();
			auto mosaics = connection[test::DatabaseName()]["mosaics"];
			size_t count = 0;
			auto view = cache.createView();
			for (auto iter = view->cbegin(); view->cend() != iter; ++iter) {
				const auto& history = iter->second;
				auto historyPosition = 0u;
				for (auto historyIter = history.cbegin(); history.cend() != historyIter; ++historyIter) {
					const auto& entry = *historyIter;

					auto filter = CreateMosaicFilter(entry.namespaceId(), entry.mosaicId(), historyPosition);
					auto document = mosaics.find_one(filter.view());
					ASSERT_TRUE(document.is_initialized());

					auto documentView = document.get().view();
					EXPECT_EQ(3u, test::GetFieldCount(documentView));
					auto descriptor = mappers::ToMosaicDescriptor(documentView);
					AssertCacheEntry(*descriptor.pEntry, entry);

					++historyPosition;
				}

				count += history.historyDepth();
			}

			EXPECT_EQ(expectedSize, count);
		}

		void AssertCanLoadAll(const std::vector<state::MosaicDescriptor>& seedDescriptors, size_t expectedCacheSize) {
			// Arrange:
			TestContext context(SeedOptions::None);
			SeedDatabase(seedDescriptors);

			// Sanity;
			EXPECT_EQ(expectedCacheSize, NumMosaicsInCollection());

			// Act:
			context.cacheStorage().loadAll(context.cache(), Height(0));

			// Assert:
			AssertCacheContents(context.cache().sub<cache::MosaicCache>(), expectedCacheSize);
		}
	}

	TEST(TEST_CLASS, LoadAllIsNoOpIfDbCollectionIsEmpty) {
		// Assert:
		AssertCanLoadAll({}, 0u);
	}

	// region history depth one

	TEST(TEST_CLASS, CanLoadAll_SingleMosaic) {
		// Arrange:
		// - CreateMosaicEntry: namespaceId / mosaicId
		// - MosaicDescriptor: mosaicEntry / index / isActive
		std::vector<state::MosaicDescriptor> seedEntries{ state::MosaicDescriptor{ CreateMosaicEntry(100, 200), 0, true } };

		// Assert:
		AssertCanLoadAll(seedEntries, 1);
	}

	TEST(TEST_CLASS, CanLoadAll_MultipleMosaics_DifferentNamespaceIds) {
		// Arrange:
		// - CreateMosaicEntry: namespaceId / mosaicId
		// - MosaicDescriptor: mosaicEntry / index / isActive
		std::vector<state::MosaicDescriptor> seedEntries{
			state::MosaicDescriptor{ CreateMosaicEntry(100, 200), 0, true },
			state::MosaicDescriptor{ CreateMosaicEntry(101, 201), 0, true },
			state::MosaicDescriptor{ CreateMosaicEntry(102, 202), 0, true }
		};

		// Assert:
		AssertCanLoadAll(seedEntries, 3);
	}

	TEST(TEST_CLASS, CanLoadAll_MultipleMosaics_SameNamespaceId) {
		// Arrange:
		// - CreateMosaicEntry: namespaceId / mosaicId
		// - MosaicDescriptor: mosaicEntry / index / isActive
		std::vector<state::MosaicDescriptor> seedEntries{
			state::MosaicDescriptor{ CreateMosaicEntry(100, 200), 0, true },
			state::MosaicDescriptor{ CreateMosaicEntry(100, 201), 0, true },
			state::MosaicDescriptor{ CreateMosaicEntry(100, 202), 0, true }
		};

		// Assert:
		AssertCanLoadAll(seedEntries, 3);
	}

	// endregion

	// region history depth greater than one

	TEST(TEST_CLASS, CanLoadAll_MultipleMosaics_SameNamespaceId_SameMosaicId) {
		// Arrange:
		// - CreateMosaicEntry: namespaceId / mosaicId
		// - MosaicDescriptor: mosaicEntry / index / isActive
		std::vector<state::MosaicDescriptor> seedEntries{
			state::MosaicDescriptor{ CreateMosaicEntry(100, 200), 0, false },
			state::MosaicDescriptor{ CreateMosaicEntry(100, 200), 1, false },
			state::MosaicDescriptor{ CreateMosaicEntry(100, 200), 2, true }
		};

		// Assert:
		AssertCanLoadAll(seedEntries, 3);
	}

	TEST(TEST_CLASS, CanLoadAll_MultipleMosaics_MultipleHistories) {
		// Arrange:
		// - CreateMosaicEntry: namespaceId / mosaicId
		// - MosaicDescriptor: mosaicEntry / index / isActive
		std::vector<state::MosaicDescriptor> seedEntries{
			state::MosaicDescriptor{ CreateMosaicEntry(100, 200), 0, false },
			state::MosaicDescriptor{ CreateMosaicEntry(100, 200), 1, false },
			state::MosaicDescriptor{ CreateMosaicEntry(100, 200), 2, true },
			state::MosaicDescriptor{ CreateMosaicEntry(101, 201), 0, false },
			state::MosaicDescriptor{ CreateMosaicEntry(101, 201), 1, true },
			state::MosaicDescriptor{ CreateMosaicEntry(102, 202), 0, true }
		};

		// Assert:
		AssertCanLoadAll(seedEntries, 6);
	}

	// endregion

	// endregion
}}}
