#include "src/storages/MongoNamespaceCacheStorage.h"
#include "src/storages/CacheStorageUtils.h"
#include "plugins/mongo/coremongo/src/MongoDbStorage.h"
#include "plugins/mongo/coremongo/src/mappers/MapperUtils.h"
#include "plugins/mongo/namespace/src/mappers/NamespaceDescriptorMapper.h"
#include "plugins/mongo/namespace/tests/test/MapperTestUtils.h"
#include "plugins/txes/namespace/src/state/RootNamespace.h"
#include "plugins/txes/namespace/tests/test/NamespaceCacheTestUtils.h"
#include "plugins/txes/namespace/tests/test/NamespaceTestUtils.h"
#include "tests/test/mongo/MapperTestUtils.h"
#include "tests/test/mongo/MongoTestUtils.h"
#include "tests/TestHarness.h"

#ifdef _WIN32
#include <boost/config/compiler/visualc.hpp>
#endif

#define TEST_CLASS MongoNamespaceCacheStorageTests

using namespace bsoncxx::builder::stream;

namespace catapult { namespace mongo { namespace storages {

	namespace {
		namespace test = catapult::test;

		// Roots have the following number of entries:
		// - id 1: 2 roots, no children
		// - id 2: 1 root, 3 children
		// - id 3: 2 roots, 3 + 3 children
		// - id 4: 2 roots, 6 + 6 (inherited) children
		constexpr size_t Num_Default_Entries = 2 + 4 + 2 * 4 + 2 * 7;
		const auto Owner1 = test::GenerateRandomData<Key_Size>();
		const auto Owner2 = test::GenerateRandomData<Key_Size>();
		const auto Owner3 = test::GenerateRandomData<Key_Size>();
		const auto Owner4 = test::GenerateRandomData<Key_Size>();
		const state::NamespaceLifetime Default_Lifetime{ Height(12), Height(23) };
		auto DoNothing = [](auto&, const auto&) {};

		using Descriptors = std::vector<state::NamespaceDescriptor>;
		using Path = state::Namespace::Path;

		struct RootNamespaceDescriptor {
		public:
			// note that the constructor is not explicit to allow an initializer list for the children
			RootNamespaceDescriptor(
					NamespaceId::ValueType id,
					const Key& owner,
					const std::vector<std::vector<NamespaceId::ValueType>>& children,
					uint32_t index = 0,
					bool isActive = true)
					: Id(id)
					, Owner(owner)
					, Children(children)
					, Index(index)
					, IsActive(isActive)
				{}

		public:
			NamespaceId::ValueType Id;
			Key Owner;
			std::vector<std::vector<NamespaceId::ValueType>> Children;
			uint32_t Index;
			bool IsActive;
		};

		Descriptors CreateNamespaceDescriptors(const std::vector<RootNamespaceDescriptor>& entries) {
			Descriptors descriptors;
			for (const auto& entry : entries) {
				auto pRoot = std::make_shared<state::RootNamespace>(NamespaceId(entry.Id), entry.Owner, Default_Lifetime);
				auto path = test::CreatePath({ entry.Id });
				descriptors.emplace_back(path, pRoot, entry.Index, entry.IsActive);
				for (const auto& child : entry.Children) {
					auto childPath = test::CreatePath(child);
					descriptors.emplace_back(childPath, pRoot, entry.Index, entry.IsActive);
				}
			}

			return descriptors;
		}

		Descriptors ExtractDescriptorsFromCache(const cache::NamespaceCache& cache) {
			Descriptors allDescriptors;
			auto view = cache.createView();
			for (auto iter = view->cbegin(); view->cend() != iter; ++iter) {
				auto descriptors = NamespaceDescriptorsFromHistory(iter->second);
				std::copy(descriptors.begin(), descriptors.end(), std::back_inserter(allDescriptors));
			}

			return allDescriptors;
		}

		void InsertIntoDelta(cache::NamespaceCacheDelta& delta, const Descriptors& descriptors) {
			for (const auto& descriptor : descriptors) {
				if (descriptor.IsRoot())
					delta.insert(*descriptor.pRoot);
				else
					delta.insert(state::Namespace(descriptor.Path));
			}
		}

		void RemoveFromDelta(cache::NamespaceCacheDelta& delta, const std::vector<NamespaceId>& idsToRemove) {
			for (auto id : idsToRemove)
				delta.remove(id);
		}

		void SeedDatabaseAndCache(cache::CatapultCache& cache, plugins::ExternalCacheStorage& storage, const Descriptors& descriptors) {
			// - seed cache
			auto delta = cache.createDelta();
			auto& namespaceCacheDelta = delta.sub<cache::NamespaceCache>();
			InsertIntoDelta(namespaceCacheDelta, descriptors);

			// - seed database
			storage.saveDelta(delta);

			// - commit
			cache.commit(Height());
		}

		void SeedDefault(cache::CatapultCache& cache, plugins::ExternalCacheStorage& storage) {
			// seed the database and the namespace cache
			// - id / owner / children / index / isActive
			std::vector<RootNamespaceDescriptor> entries{
				{ 1, Owner1, {}, 0, true }, // multiple, no children
				{ 1, Owner1, {}, 1, true },
				{ 2, Owner2, { { 2, 20 }, { 2, 20, 21 }, { 2, 22 } }, 0, true }, // single, children
				{ 3, Owner3, { { 3, 30 }, { 3, 30, 31 }, { 3, 32 } }, 0, false }, // multiple, children,
				{ 3, test::GenerateRandomData<Key_Size>(), { { 3, 33 }, { 3, 33, 34 }, { 3, 35 } }, 1, true }, // diff owner
				{ 4, Owner4, { { 4, 40 }, { 4, 40, 41 }, { 4, 42 } }, 0, false }, // multiple, children, same owner
				{ 4, Owner4, { { 4, 43 }, { 4, 43, 44 }, { 4, 45 } }, 1, true }
			};
			auto descriptors = CreateNamespaceDescriptors(entries);
			SeedDatabaseAndCache(cache, storage, descriptors);
		}

		int64_t NumNamespacesInCollection() {
			auto connection = test::CreateDbConnection();
			auto namespaces = connection[test::DatabaseName()]["namespaces"];
			return namespaces.count({});
		}

		enum class SeedOptions { None, Default };

		class TestContext final : public test::PrepareDatabaseMixin {
		public:
			explicit TestContext(SeedOptions seedOptions)
					: PrepareDatabaseMixin(test::DatabaseName())
					, m_pConfig(test::CreateDefaultMongoStorageConfiguration(test::DatabaseName()))
					, m_pCacheStorage(CreateMongoNamespaceCacheStorage(m_pConfig->createDatabaseConnection(), m_pConfig->bulkWriter()))
					, m_cache(test::NamespaceCacheFactory::Create()) {
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
				EXPECT_EQ(Num_Default_Entries, NumNamespacesInCollection());
			}

		private:
			std::shared_ptr<plugins::MongoStorageConfiguration> m_pConfig;
			std::unique_ptr<plugins::ExternalCacheStorage> m_pCacheStorage;
			cache::CatapultCache m_cache;
		};

		bsoncxx::document::value CreateNamespacePathFilter(const Path& path, uint32_t index) {
			document doc{};
			auto filter = doc
					<< "$and"
					<< open_array
						<< open_document
							<< "namespace.level0" << mappers::ToInt64(path[0])
						<< close_document;

			if (1 < path.size()) {
				filter
						<< open_document
							<< "namespace.level1" << mappers::ToInt64(path[1])
						<< close_document;
			}

			if (2 < path.size()) {
				filter
						<< open_document
							<< "namespace.level2" << mappers::ToInt64(path[2])
						<< close_document;
			}

			filter
						<< open_document
							<< "meta.index" << static_cast<int32_t>(index)
						<< close_document
						<< open_document
							<< "namespace.depth" << static_cast<int32_t>(path.size())
						<< close_document
					<< close_array;

			return doc << finalize;
		}

		void AssertMatch(const bsoncxx::document::value& filter, const state::NamespaceDescriptor& descriptor) {
			auto connection = test::CreateDbConnection();
			auto database = connection[test::DatabaseName()];
			auto dbNamespace = database["namespaces"].find_one(filter.view());
			ASSERT_TRUE(dbNamespace.is_initialized());

			auto view = dbNamespace.get().view();
			EXPECT_EQ(3u, test::GetFieldCount(view));

			auto metaView = view["meta"].get_document().view();
			EXPECT_EQ(2u, test::GetFieldCount(metaView));
			mongo::test::AssertEqualNamespaceMetadata(descriptor, metaView);

			auto namespaceView = view["namespace"].get_document().view();
			EXPECT_EQ(6 + descriptor.Path.size(), test::GetFieldCount(namespaceView));
			mongo::test::AssertEqualNamespaceData(descriptor, namespaceView);
		}

		void AssertDbNamespaces(const Descriptors& descriptors) {
			for (const auto& descriptor : descriptors) {
				auto filter = CreateNamespacePathFilter(descriptor.Path, descriptor.Index);
				AssertMatch(filter, descriptor);
			}
		}

		void AssertCanSaveDelta(
				const std::vector<RootNamespaceDescriptor>& entriesToInsert,
				const std::vector<NamespaceId>& idsToRemove,
				size_t expectedCollectionSize,
				std::function<void (cache::NamespaceCacheDelta&, const Descriptors&)> insertAction,
				std::function<void (cache::NamespaceCacheDelta&, const std::vector<NamespaceId>&)> removeAction,
				SeedOptions seedOptions = SeedOptions::None) {
			// Arrange:
			TestContext context(seedOptions);

			// - prepare cache delta
			auto& cache = context.cache();
			auto delta = cache.createDelta();
			auto& namespaceCacheDelta = delta.sub<cache::NamespaceCache>();
			auto descriptors = CreateNamespaceDescriptors(entriesToInsert);
			insertAction(namespaceCacheDelta, descriptors);
			removeAction(namespaceCacheDelta, idsToRemove);

			// Act:
			context.cacheStorage().saveDelta(delta);
			cache.commit(Height(0));

			// Assert:
			auto expectedDescriptors = ExtractDescriptorsFromCache(cache.sub<cache::NamespaceCache>());
			EXPECT_EQ(expectedCollectionSize, NumNamespacesInCollection());
			EXPECT_EQ(expectedCollectionSize, expectedDescriptors.size());
			AssertDbNamespaces(expectedDescriptors);
		}
	}

	// region saveDelta

	TEST(TEST_CLASS, SaveDeltaIsNoOpIfCacheIsClean) {
		// Assert:
		AssertCanSaveDelta({}, {}, 0, DoNothing, DoNothing);
	}

	// region added histories

	TEST(TEST_CLASS, CanSaveSingleRootNamespace_New_WithoutChildren) {
		// Arrange: id / owner / children / index / isActive
		std::vector<RootNamespaceDescriptor> entries{ { 123, test::GenerateRandomData<Key_Size>(), {}, 0, true } };

		// Assert:
		AssertCanSaveDelta(entries, {}, 1, InsertIntoDelta, DoNothing);
	}

	TEST(TEST_CLASS, CanSaveSingleRootNamespace_New_WithChildren) {
		// Arrange: id / owner / children / index / isActive
		std::vector<RootNamespaceDescriptor> entries{
			{ 123, test::GenerateRandomData<Key_Size>(), { { 123, 124}, { 123, 124, 125 }, { 123, 126 } }, 0, true }
		};

		// Assert:
		AssertCanSaveDelta(entries, {}, 4, InsertIntoDelta, DoNothing);
	}

	TEST(TEST_CLASS, CanSaveMultipleRootNamespaces_New_WithoutChildren) {
		// Arrange: id / owner / children / index / isActive
		std::vector<RootNamespaceDescriptor> entries{
			{ 123, test::GenerateRandomData<Key_Size>(), {}, 0, true },
			{ 234, test::GenerateRandomData<Key_Size>(), {}, 0, true },
			{ 345, test::GenerateRandomData<Key_Size>(), {}, 0, true }
		};

		// Assert:
		AssertCanSaveDelta(entries, {}, 3, InsertIntoDelta, DoNothing);
	}

	TEST(TEST_CLASS, CanSaveMultipleRootNamespaces_New_WithChildren) {
		// Arrange: id / owner / children / index / isActive
		std::vector<RootNamespaceDescriptor> entries{
			{ 123, test::GenerateRandomData<Key_Size>(), { { 123, 124 }, { 123, 124, 125 }, { 123, 126 } }, 0, true },
			{ 234, test::GenerateRandomData<Key_Size>(), { { 234, 235 }, { 234, 235, 236 } }, 0, true },
			{ 345, test::GenerateRandomData<Key_Size>(), { { 345, 346 } }, 0, true }
		};

		// Assert:
		AssertCanSaveDelta(entries, {}, 4 + 3 + 2, InsertIntoDelta, DoNothing);
	}

	// endregion

	// region modified histories

	// region modifying by adding

	TEST(TEST_CLASS, CanSaveSingleRootNamespace_ModifiedByAdding_WithoutChildren) {
		// Arrange: id / owner / children / index / isActive
		// - note that root with id 1 already exists in the namespace cache
		std::vector<RootNamespaceDescriptor> entries{ { 1, test::GenerateRandomData<Key_Size>(), {}, 1, true } };

		// Assert: collection size has increased by 1
		AssertCanSaveDelta(entries, {}, Num_Default_Entries + 1, InsertIntoDelta, DoNothing, SeedOptions::Default);
	}

	TEST(TEST_CLASS, CanSaveSingleRootNamespace_ModifiedByAdding_WithChildren) {
		// Arrange: id / owner / children / index / isActive
		// - note that root with id 1 already exists in the namespace cache
		std::vector<RootNamespaceDescriptor> entries{
			{ 1, test::GenerateRandomData<Key_Size>(), { { 1, 10 }, { 1, 10, 11 }, { 1, 12 } }, 1, true } };

		// Assert: collection size has increased by 4
		AssertCanSaveDelta(entries, {}, Num_Default_Entries + 4, InsertIntoDelta, DoNothing, SeedOptions::Default);
	}

	TEST(TEST_CLASS, CanSaveSingleRootNamespace_ModifiedByAdding_InheritedChildren) {
		// Arrange: id / owner / children / index / isActive
		// - note that root with id 2 already exists in the namespace cache and has three children
		std::vector<RootNamespaceDescriptor> entries{ { 2, Owner2, {}, 1, true } };

		// Assert: 3 children are inherited
		AssertCanSaveDelta(entries, {}, Num_Default_Entries + 4, InsertIntoDelta, DoNothing, SeedOptions::Default);
	}

	TEST(TEST_CLASS, CanSaveMultipleRootNamespaces_ModifiedByAdding_WithoutChildren) {
		// Arrange: id / owner / children / index / isActive
		std::vector<RootNamespaceDescriptor> entries{
			{ 1, test::GenerateRandomData<Key_Size>(), {}, 1, true },
			{ 2, test::GenerateRandomData<Key_Size>(), {}, 1, true },
			{ 3, test::GenerateRandomData<Key_Size>(), {}, 2, true }
		};

		// Assert: no children are inherited
		AssertCanSaveDelta(entries, {}, Num_Default_Entries + 3, InsertIntoDelta, DoNothing, SeedOptions::Default);
	}

	TEST(TEST_CLASS, CanSaveMultipleRootNamespaces_ModifiedByAdding_WithChildren) {
		// Arrange: id / owner / children / index / isActive
		std::vector<RootNamespaceDescriptor> entries{
			{ 1, test::GenerateRandomData<Key_Size>(), { { 1, 13 }, { 1, 13, 14 }, { 1, 15 } }, 1, true },
			{ 2, test::GenerateRandomData<Key_Size>(), { { 2, 23 } }, 1, true },
			{ 3, test::GenerateRandomData<Key_Size>(), { { 3, 33 }, { 3, 33, 34 } }, 2, true }
		};

		// Assert: no children are inherited
		AssertCanSaveDelta(entries, {}, Num_Default_Entries + 4 + 2 + 3, InsertIntoDelta, DoNothing, SeedOptions::Default);
	}

	TEST(TEST_CLASS, CanSaveMultipleRootNamespaces_ModifiedByAdding_InheritedChildren) {
		// Arrange: id / owner / children / index / isActive
		std::vector<RootNamespaceDescriptor> entries{
			{ 2, Owner2, {}, 1, true },
			{ 4, Owner4, {}, 2, true }
		};

		// Assert: 3 + 6 children are inherited
		AssertCanSaveDelta(entries, {}, Num_Default_Entries + 2 + 3 + 6, InsertIntoDelta, DoNothing, SeedOptions::Default);
	}

	// endregion

	// region modifying by removing partial history

	TEST(TEST_CLASS, CanSaveSingleRootNamespace_ModifiedByRemovingPartialHistory_WithoutChildren) {
		// Assert: note that root with id 1 already exists in the namespace cache with history size 2
		// - collection size has decreased by 1 (1 root and 0 children were removed)
		AssertCanSaveDelta({}, { NamespaceId(1) }, Num_Default_Entries - 1, DoNothing, RemoveFromDelta, SeedOptions::Default);
	}

	TEST(TEST_CLASS, CanSaveSingleRootNamespace_ModifiedByRemovingPartialHistory__NoInheritedChildren) {
		// Assert: note that root with id 3 already exists in the namespace cache with history size 2
		// - collection size has decreased by 4 (1 root and 3 children were removed)
		AssertCanSaveDelta({}, { NamespaceId(3) }, Num_Default_Entries - 4, DoNothing, RemoveFromDelta, SeedOptions::Default);
	}

	TEST(TEST_CLASS, CanSaveSingleRootNamespace_ModifiedByRemovingPartialHistory__InheritedChildren) {
		// Assert: note that root with id 4 already exists in the namespace cache with history size 2
		// - note that the 2 histories entries have the same owner, the most recent entry has inherited 3 children
		// - collection size has decreased by 7 (1 root and 6 children were removed)
		AssertCanSaveDelta({}, { NamespaceId(4) }, Num_Default_Entries - 7, DoNothing, RemoveFromDelta, SeedOptions::Default);
	}

	TEST(TEST_CLASS, CanSaveMultipleRootNamespaces_ModifiedByRemovingPartialHistory__WithAndWithoutInheritedChildren) {
		// Assert: note that roots with id 3 and 4 already exist in the namespace cache with history size 2
		// - collection size has decreased by 4 + 7 (2 roots and 9 children were removed)
		AssertCanSaveDelta(
				{},
				{ NamespaceId(3), NamespaceId(4) },
				Num_Default_Entries - 11,
				DoNothing,
				RemoveFromDelta,
				SeedOptions::Default);
	}

	// endregion

	// endregion

	// region removing full history

	TEST(TEST_CLASS, CanSaveSingleRootNamespace_RemovingFullHistory_WithoutChildren) {
		// Assert: note that root with id 1 already exists in the namespace cache with history size 1
		// - collection size has decreased by 1 (1 root and 0 children were removed)
		AssertCanSaveDelta({}, { NamespaceId(1) }, Num_Default_Entries - 1, DoNothing, RemoveFromDelta, SeedOptions::Default);
	}

	TEST(TEST_CLASS, CanSaveSingleRootNamespace_RemovingFullHistory_WithChildren) {
		// Assert: note that root with id 2 already exists in the namespace cache with history size 1
		// - first remove children, then root
		// - collection size has decreased by 4 (1 root and 3 children were removed)
		AssertCanSaveDelta(
				{},
				{ NamespaceId(22), NamespaceId(21), NamespaceId(20), NamespaceId(2) },
				Num_Default_Entries - 4,
				DoNothing,
				RemoveFromDelta,
				SeedOptions::Default);
	}

	TEST(TEST_CLASS, CanSaveMultipleRootNamespaces_RemovingFullHistory_WithAndWithoutChildren) {
		// Assert: note that
		// - root with id 1 already exists in the namespace cache with history size 2
		// - root with id 2 already exists in the namespace cache with history size 1
		// - first remove children, then root
		// - collection size has decreased by 6 (3 roots and 3 children were removed)
		AssertCanSaveDelta(
				{},
				{ NamespaceId(1), NamespaceId(1), NamespaceId(22), NamespaceId(21), NamespaceId(20), NamespaceId(2) },
				Num_Default_Entries - 6,
				DoNothing,
				RemoveFromDelta,
				SeedOptions::Default);
	}

	// endregion

	// region mixed operations

	TEST(TEST_CLASS, CanSaveMultipleRootNamespaces_Added_Modified_Removed) {
		// Arrange: id / owner / children / index / isActive
		std::vector<RootNamespaceDescriptor> entries{
			{ 123, test::GenerateRandomData<Key_Size>(), { { 123, 124 }, { 123, 126 } }, 0, true }, // added
			{ 1, test::GenerateRandomData<Key_Size>(), { { 1, 10 } }, 1, true } // modified by adding
		};

		// Assert: note that
		// - root with id 1 already exists in the namespace cache with history size 2 (3 after add)
		// - root with id 2 already exists in the namespace cache with history size 1
		// - first remove children, then root
		// - collection size changes:
		//   +3 by adding namespace with id 123 (1 root + 2 children)
		//   +2 by adding namespace with id 1 (1 root + 1 child)
		//   -2 by removing previously added namespace with id 1 (1 root + 1 child)
		//   -4 by removing namespace with id 2 (1 root + 3 children)
		AssertCanSaveDelta(
				entries,
				{ NamespaceId(1), NamespaceId(22), NamespaceId(21), NamespaceId(20), NamespaceId(2) }, // partially removed / removed
				Num_Default_Entries + 3 + 2 - 2 - 4,
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
			auto namespaces = connection[test::DatabaseName()]["namespaces"];
			for (const auto& descriptor : descriptors) {
				auto dbNamespace = mappers::ToDbModel(descriptor);
				namespaces.insert_one(dbNamespace.view());
			}
		}

		void AssertCacheEntry(
				const Path& path,
				const state::RootNamespace& root,
				bool isActive,
				const state::NamespaceDescriptor& descriptor) {
			EXPECT_EQ(descriptor.IsActive, isActive);
			EXPECT_EQ(descriptor.Path, path);
			EXPECT_EQ(descriptor.pRoot->id(), root.id());
			EXPECT_EQ(descriptor.pRoot->owner(), root.owner());
			EXPECT_EQ(descriptor.pRoot->lifetime(), root.lifetime());
		}

		void AssertCacheContents(const cache::NamespaceCache& cache, size_t expectedSize) {
			auto connection = test::CreateDbConnection();
			auto namespaces = connection[test::DatabaseName()]["namespaces"];
			size_t count = 0;
			auto view = cache.createView();
			for (auto iter = view->cbegin(); view->cend() != iter; ++iter) {
				const auto& history = iter->second;
				auto historyPosition = 0u;
				for (auto historyIter = history.cbegin(); history.cend() != historyIter; ++historyIter) {
					const auto& root = *historyIter;

					// - root
					auto rootPath = test::CreatePath({ root.id().unwrap() });
					auto rootFilter = CreateNamespacePathFilter(rootPath, historyPosition);
					auto dbRoot = namespaces.find_one(rootFilter.view());
					ASSERT_TRUE(dbRoot.is_initialized());
					auto rootDescriptor = mappers::ToNamespaceDescriptor(dbRoot.get());
					AssertCacheEntry(rootPath, root, history.historyDepth() - 1 == historyPosition, rootDescriptor);

					// - children
					for (const auto& pair : root.children()) {
						auto childFilter = CreateNamespacePathFilter(pair.second, historyPosition);
						auto dbChild = namespaces.find_one(childFilter.view());
						ASSERT_TRUE(dbChild.is_initialized());
						auto childDescriptor = mappers::ToNamespaceDescriptor(dbChild.get());
						AssertCacheEntry(pair.second, root, history.historyDepth() - 1 == historyPosition, childDescriptor);
					}

					++historyPosition;
				}

				count += history.historyDepth() + history.numAllHistoricalChildren();
			}

			EXPECT_EQ(expectedSize, count);
		}

		void AssertCanLoadAll(const std::vector<RootNamespaceDescriptor>& seedEntries, size_t expectedCacheSize) {
			// Arrange:
			TestContext context(SeedOptions::None);
			auto descriptors = CreateNamespaceDescriptors(seedEntries);
			SeedDatabase(descriptors);

			// Sanity;
			EXPECT_EQ(expectedCacheSize, NumNamespacesInCollection());

			// Act:
			context.cacheStorage().loadAll(context.cache(), Height(0));

			// Assert:
			AssertCacheContents(context.cache().sub<cache::NamespaceCache>(), expectedCacheSize);
		}
	}

	TEST(TEST_CLASS, LoadAllIsNoOpIfDbCollectionIsEmpty) {
		// Assert:
		AssertCanLoadAll({}, 0u);
	}

	// region history depth one

	TEST(TEST_CLASS, CanLoadAll_SingleRootNamespace_WithoutChildren) {
		// Arrange: id / owner / children / index / isActive
		std::vector<RootNamespaceDescriptor> seedEntries{ { 1, test::GenerateRandomData<Key_Size>(), {}, 0, true } };

		// Assert:
		AssertCanLoadAll(seedEntries, 1);
	}

	TEST(TEST_CLASS, CanLoadAll_SingleRootNamespace_WithChildren) {
		// Arrange: id / owner / children / index / isActive
		std::vector<RootNamespaceDescriptor> seedEntries{
			{ 1, test::GenerateRandomData<Key_Size>(), { { 1, 10 }, { 1, 10, 11 }, { 1, 12 } }, 0, true } };

		// Assert:
		AssertCanLoadAll(seedEntries, 4u);
	}

	TEST(TEST_CLASS, CanLoadAll_MultipleRootNamespaces_WithoutChildren) {
		// Arrange: id / owner / children / index / isActive
		std::vector<RootNamespaceDescriptor> seedEntries{
			{ 1, test::GenerateRandomData<Key_Size>(), {}, 0, true },
			{ 2, test::GenerateRandomData<Key_Size>(), {}, 0, true },
			{ 3, test::GenerateRandomData<Key_Size>(), {}, 0, true },
		};

		// Assert:
		AssertCanLoadAll(seedEntries, 3);
	}

	TEST(TEST_CLASS, CanLoadAll_MultipleRootNamespaces_WithChildren) {
		// Arrange: id / owner / children / index / isActive
		std::vector<RootNamespaceDescriptor> seedEntries{
			{ 1, test::GenerateRandomData<Key_Size>(), { { 1, 10 } }, 0, true },
			{ 2, test::GenerateRandomData<Key_Size>(), { { 2, 20 }, { 2, 20, 21 }, { 2, 22 } }, 0, true },
			{ 3, test::GenerateRandomData<Key_Size>(), { { 3, 30 }, { 3, 30, 31 } }, 0, true },
		};

		// Assert:
		AssertCanLoadAll(seedEntries, 2 + 4 + 3);
	}

	// endregion

	// region history depth greater than one

	TEST(TEST_CLASS, CanLoadAll_DifferentOwners_WithoutChildren) {
		// Arrange: id / owner / children / index / isActive
		std::vector<RootNamespaceDescriptor> seedEntries{
			{ 1, test::GenerateRandomData<Key_Size>(), {}, 0, false },
			{ 1, test::GenerateRandomData<Key_Size>(), {}, 1, false },
			{ 1, test::GenerateRandomData<Key_Size>(), {}, 2, true },
		};

		// Assert:
		AssertCanLoadAll(seedEntries, 3);
	}

	TEST(TEST_CLASS, CanLoadAll_DifferentOwners_WithChildren) {
		// Arrange: id / owner / children / index / isActive
		std::vector<RootNamespaceDescriptor> seedEntries{
			{ 1, test::GenerateRandomData<Key_Size>(), { { 1, 10 }, { 1, 10, 11 }, { 1, 12 } }, 0, false },
			{ 1, test::GenerateRandomData<Key_Size>(), { { 1, 13 }, { 1, 13, 14 } }, 1, false },
			{ 1, test::GenerateRandomData<Key_Size>(), { { 1, 15 } }, 2, true },
		};

		// Assert: no children are inherited
		AssertCanLoadAll(seedEntries, 4 + 3 + 2);
	}

	TEST(TEST_CLASS, CanLoadAll_SameOwner_WithoutChildren) {
		// Arrange: id / owner / children / index / isActive
		std::vector<RootNamespaceDescriptor> seedEntries{
			{ 1, Owner1, {}, 0, false },
			{ 1, Owner1, {}, 1, false },
			{ 1, Owner1, {}, 2, true },
		};

		// Assert:
		AssertCanLoadAll(seedEntries, 3);
	}

	TEST(TEST_CLASS, CanLoadAll_SameOwner_WithChildren) {
		// Arrange: id / owner / children / index / isActive
		// - note that this simulates the following sequence:
		//   1) first root is add, then children with ids { 10, 11, 12 } are added
		//   2) second root is added, then child with id { 13 } is added
		//   3) third root is added, then  child with id { 14 } is added
		//   Since roots with same owner share children, all roots refer to the same set of children
		std::vector<RootNamespaceDescriptor> seedEntries{
			{ 1, Owner1, { { 1, 10 }, { 1, 10, 11 }, { 1, 12 }, { 1, 12, 13 }, { 1, 14 } }, 0, false },
			{ 1, Owner1, { { 1, 10 }, { 1, 10, 11 }, { 1, 12 }, { 1, 12, 13 }, { 1, 14 } }, 1, false },
			{ 1, Owner1, { { 1, 10 }, { 1, 10, 11 }, { 1, 12 }, { 1, 12, 13 }, { 1, 14 } }, 2, true },
		};

		// Assert:
		AssertCanLoadAll(seedEntries, 6 + 6 + 6);
	}

	TEST(TEST_CLASS, CanLoadAll_AlternatingOwner_WithChildren) {
		// Arrange: id / owner / children / index / isActive
		// - note that since the owner changes from root to root, there is no inheritance of children
		std::vector<RootNamespaceDescriptor> seedEntries{
			{ 1, Owner1, { { 1, 10 }, { 1, 10, 11 }, { 1, 12 } }, 0, false },
			{ 1, test::GenerateRandomData<Key_Size>(), { { 1, 13 } }, 1, false },
			{ 1, Owner1, { { 1, 15 }, { 1, 15, 16 } }, 2, true },
		};

		// Assert:
		AssertCanLoadAll(seedEntries, 4 + 2 + 3);
	}

	// endregion

	// endregion
}}}
