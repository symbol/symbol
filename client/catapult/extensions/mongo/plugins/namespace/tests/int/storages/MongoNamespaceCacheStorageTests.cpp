#include "src/storages/MongoNamespaceCacheStorage.h"
#include "src/mappers/NamespaceDescriptor.h"
#include "mongo/src/mappers/MapperUtils.h"
#include "catapult/model/Address.h"
#include "mongo/tests/test/MongoHistoricalCacheStorageTests.h"
#include "mongo/tests/test/MongoTestUtils.h"
#include "plugins/txes/namespace/tests/test/NamespaceCacheTestUtils.h"
#include "plugins/txes/namespace/tests/test/NamespaceTestUtils.h"
#include "tests/test/NamespaceMapperTestUtils.h"
#include "tests/TestHarness.h"

using namespace bsoncxx::builder::stream;

namespace catapult { namespace mongo { namespace plugins {

#define TEST_CLASS MongoNamespaceCacheStorageTests

	namespace {
		using Path = state::Namespace::Path;

		bsoncxx::document::value CreateNamespacePathFilter(const Path& path, uint32_t index) {
			document doc;
			auto filter = doc
					<< "$and"
					<< open_array
					<< open_document << "namespace.level0" << mappers::ToInt64(path[0]) << close_document;

			if (1 < path.size())
				filter << open_document << "namespace.level1" << mappers::ToInt64(path[1]) << close_document;

			if (2 < path.size())
				filter << open_document << "namespace.level2" << mappers::ToInt64(path[2]) << close_document;

			filter
					<< open_document << "meta.index" << static_cast<int32_t>(index) << close_document
					<< open_document << "namespace.depth" << static_cast<int32_t>(path.size()) << close_document
					<< close_array;

			return doc << finalize;
		}

		Path CreateRootPath(NamespaceId id) {
			Path path;
			path.push_back(id);
			return path;
		}

		struct NamespaceCacheTraits {
			using CacheType = cache::NamespaceCache;
			using ModelType = NamespaceDescriptor;

			static constexpr auto Collection_Name = "namespaces";
			static constexpr auto Network_Id = static_cast<model::NetworkIdentifier>(0x5A);
			static constexpr auto CreateCacheStorage = CreateMongoNamespaceCacheStorage;

			static cache::CatapultCache CreateCache() {
				return test::NamespaceCacheFactory::Create();
			}

			static NamespaceDescriptor GenerateRandomElement(uint32_t id, uint32_t index, bool isActive) {
				return CreateElement(test::GenerateRandomData<Key_Size>(), id, index, isActive);
			}

			static NamespaceDescriptor CreateElement(const Key& key, uint32_t id, uint32_t index, bool isActive) {
				auto pRoot = std::make_shared<state::RootNamespace>(NamespaceId(id), key, test::CreateLifetime(123, 456));
				auto address = model::PublicKeyToAddress(key, Network_Id);
				return NamespaceDescriptor(CreateRootPath(NamespaceId(id)), pRoot, address, index, isActive);
			}

			static NamespaceDescriptor CreateElement(const state::RootNamespace& root, uint32_t index, bool isActive) {
				auto pRootCopy = std::make_shared<state::RootNamespace>(root);
				auto address = model::PublicKeyToAddress(root.owner(), Network_Id);
				return NamespaceDescriptor(CreateRootPath(pRootCopy->id()), pRootCopy, address, index, isActive);
			}

			static void Add(cache::CatapultCacheDelta& delta, const ModelType& descriptor) {
				auto& namespaceCacheDelta = delta.sub<cache::NamespaceCache>();
				namespaceCacheDelta.insert(*descriptor.pRoot);
			}

			static void Remove(cache::CatapultCacheDelta& delta, const ModelType& descriptor) {
				auto& namespaceCacheDelta = delta.sub<cache::NamespaceCache>();
				namespaceCacheDelta.remove(descriptor.pRoot->id());
			}

			static auto GetFindFilter(const ModelType& descriptor) {
				return CreateNamespacePathFilter(descriptor.Path, descriptor.Index);
			}

			static void AssertEqual(const ModelType& descriptor, const bsoncxx::document::view& view) {
				test::AssertEqualNamespaceData(descriptor, view["namespace"].get_document().view());
			}
		};
	}

	struct NamespaceCacheRootModificationTraits : public NamespaceCacheTraits {
		static NamespaceDescriptor Mutate(cache::CatapultCacheDelta& delta, ModelType& descriptor) {
			// change owner
			auto key = test::GenerateRandomData<Key_Size>();
			auto pChangedRoot = std::make_shared<state::RootNamespace>(descriptor.pRoot->id(), key, descriptor.pRoot->lifetime());
			auto address = model::PublicKeyToAddress(key, Network_Id);

			// update database and return new descriptor
			auto modifiedDescriptor = NamespaceDescriptor(descriptor.Path, pChangedRoot, address, descriptor.Index + 1, true);
			Add(delta, modifiedDescriptor);
			return modifiedDescriptor;
		}
	};

	struct NamespaceCacheChildModificationTraits : public NamespaceCacheTraits {
		static NamespaceDescriptor Mutate(cache::CatapultCacheDelta& delta, ModelType& descriptor) {
			// insert a child
			auto childPath = descriptor.Path;
			childPath.push_back(NamespaceId(987));

			// update database and return new descriptor
			auto& namespaceCacheDelta = delta.sub<cache::NamespaceCache>();
			namespaceCacheDelta.insert(state::Namespace(childPath));
			return NamespaceDescriptor(childPath, descriptor.pRoot, descriptor.OwnerAddress, descriptor.Index, true);
		}
	};

	// modifications that create historical entries (increasing indexes)
	DEFINE_HISTORICAL_CACHE_STORAGE_TESTS(NamespaceCacheRootModificationTraits, _RootModification);

	// modifications that create children (path filter is passed path with multiple parts)
	DEFINE_HISTORICAL_CACHE_STORAGE_TESTS(NamespaceCacheChildModificationTraits, _ChildModification);

	// region custom load tests

	namespace {
		struct RootNamespaceDescriptor {
		private:
			using ChildrenPaths = std::vector<std::vector<NamespaceId::ValueType>>;

		public:
			// note that the constructor is not explicit to allow an initializer list for the children
			RootNamespaceDescriptor(NamespaceId::ValueType id, const Key& owner, const ChildrenPaths& paths)
					: Id(id)
					, Owner(owner)
					, Children(paths)
				{}

		public:
			NamespaceId::ValueType Id;
			Key Owner;
			ChildrenPaths Children;
		};

		void AssertCanLoadAll(const std::vector<RootNamespaceDescriptor>& seeds, size_t expectedCacheSize, size_t expectedDeepSize) {
			// Arrange:
			using Runner = test::MongoHistoricalCacheStorageTests<NamespaceCacheRootModificationTraits>;
			Runner::RunCustomLoadTest([&seeds, expectedCacheSize, expectedDeepSize](auto& delta) {
				auto& namespaceCacheDelta = delta.template sub<cache::NamespaceCache>();

				auto lifetime = test::CreateLifetime(123, 456);
				for (const auto& rootDescriptor : seeds) {
					auto pRoot = std::make_shared<state::RootNamespace>(NamespaceId(rootDescriptor.Id), rootDescriptor.Owner, lifetime);
					namespaceCacheDelta.insert(*pRoot);

					for (const auto& childPath : rootDescriptor.Children)
						namespaceCacheDelta.insert(state::Namespace(test::CreatePath(childPath)));
				}

				return std::make_pair(expectedCacheSize, expectedDeepSize);
			});
		}
	}

	TEST(TEST_CLASS, CanLoadAll_SameOwner_WithChildren) {
		// Arrange: id / owner / children
		// - note that this simulates the following sequence:
		//   1) first root is added, then children with ids { 10, 11, 12 } are added
		//   2) second root is added, then child with id { 13 } is added
		//   3) third root is added, then  child with id { 14 } is added
		//   since roots with same owner share children, all roots refer to the same set of children
		auto owner = test::GenerateRandomData<Key_Size>();
		std::vector<RootNamespaceDescriptor> seedEntries{
			{ 1, owner, { { 1, 10 }, { 1, 10, 11 }, { 1, 12 } } },
			{ 1, owner, { { 1, 12, 13 } } },
			{ 1, owner, { { 1, 14 } } }
		};

		// Assert:
		AssertCanLoadAll(seedEntries, 3, 6 + 6 + 6);
	}

	TEST(TEST_CLASS, CanLoadAll_AlternatingOwner_WithChildren) {
		// Arrange: id / owner / children
		// - note that since the owner changes from root to root, there is no inheritance of children
		auto owner1 = test::GenerateRandomData<Key_Size>();
		auto owner2 = test::GenerateRandomData<Key_Size>();
		std::vector<RootNamespaceDescriptor> seedEntries{
			{ 1, owner1, { { 1, 10 }, { 1, 10, 11 }, { 1, 12 } } },
			{ 1, owner2, { { 1, 13 } } },
			{ 1, owner1, { { 1, 15 }, { 1, 15, 16 } } },
		};

		// Assert:
		AssertCanLoadAll(seedEntries, 3, 4 + 2 + 3);
	}

	// endregion
}}}
